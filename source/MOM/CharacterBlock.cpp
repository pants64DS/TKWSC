#include "CharacterBlock.h"

// Overlaps with HEALTH_ARR
extern u8 PLAYER_HEALTH;
asm("PLAYER_HEALTH = 0x02092145");

using CharacterBlockCLPS = StaticCLPS_Block<
{
	.camBehavID = CLPS::CA_NORMAL,
	.camGoesThru = true
}>;

static const uint8_t SET_BIT_AMOUNT_LOOKUP [16] =
{
	0, 1, 1, 2, 1, 2, 2, 3, 
	1, 2, 2, 3, 2, 3, 3, 4
};

SharedFilePtr CharacterBlock::modelFiles[2];
SharedFilePtr CharacterBlock::texSeqFiles[3];
SharedFilePtr CharacterBlock::clsnFile;
SharedFilePtr CharacterBlock::animFile;

bool CharacterBlock::hasYoshiUpdated;
int CharacterBlock::currentYoshi;
int CharacterBlock::nextYoshi;

SpawnInfo CharacterBlock::spawnData =
{
	[] -> ActorBase* { return new CharacterBlock; },
	0x0034,
	0x0100,
	0x00000002,
	0x00000000_f,
	0x005dc000_f,
	0x01000000_f,
	0x01000000_f
};

[[gnu::target("thumb")]]
unsigned CharacterBlock::CountStars()
{
	unsigned starCount = 0;
	for (int i = 0; i < 30; i++)
		starCount += SET_BIT_AMOUNT_LOOKUP[SAVE_DATA.stars[i] & 0x0F] + SET_BIT_AMOUNT_LOOKUP[SAVE_DATA.stars[i] >> 4];
	
	return starCount;
}

bool CharacterBlock::CheckUnlock()
{
	//Check if the player has unlocked the character if they need to be unlocked
	if (needsUnlock && blockType != startingCharacter)
	{
		//First check if the player is Yoshi (code only gets reached if the starting character has been changed) and then check if the flag for unlocking the starting character = 0
		if (blockType == 3 && (SAVE_DATA.flags2 & 0xff & 1 << startingCharacter) == 0)
			return false;
		//If the player is not Yoshi check if the flag for the blocktype character = 0
		else if ((SAVE_DATA.flags2 & 0xff & 1 << blockType) == 0)
			return false;
	}
	
	return true;
}

[[gnu::target("thumb")]]
int CharacterBlock::CheckYoshi()
{
	if (blockType != 3)
		return currentYoshi;

	return SAVE_DATA.cannonsUnlocked >> 8 & 3;
}

[[gnu::target("thumb")]]
void CharacterBlock::JumpedUnderBlock()
{
	// check if the player is the same character as the blocktype and isn't Yoshi
	if (SAVE_DATA.currentCharacter == blockType && SAVE_DATA.currentCharacter != 3)
		return;
	// check if blockType is Yoshi, the Player is Yoshi and the highest unlocked Yoshi is green
	if (blockType == 3 && SAVE_DATA.currentCharacter == 3 && highestUnlockedYoshi == 0)
		return;
	// check if the Player is riding Yoshi
	if (PLAYER_ARR[0]->param1 == 3 && SAVE_DATA.currentCharacter != 3)
		return;
	
	// if it's a Yoshi Block
	if (blockType == 3)
	{
		// if you're Yoshi switching to another Yoshi
		if (SAVE_DATA.currentCharacter == 3)
		{
			currentYoshi++;
			if (currentYoshi > highestUnlockedYoshi)
				currentYoshi = 0;
			
			nextYoshi = currentYoshi + 1;
			if (nextYoshi > highestUnlockedYoshi)
				nextYoshi = 0;

			SAVE_DATA.cannonsUnlocked &= ~0x300;
			SAVE_DATA.cannonsUnlocked |= (currentYoshi & 3) << 8;
		}
		else // if you're another character switching to Yoshi
		{
			currentYoshi = nextYoshi;
			nextYoshi++;
			if (nextYoshi > highestUnlockedYoshi)
				nextYoshi = 0;
		}
	} // if you're Yoshi switching to another character
	else if (SAVE_DATA.currentCharacter == 3)
	{
		nextYoshi = currentYoshi;
	}
	
	// change the character into the character of the blocktype
	uint8_t prev_health = PLAYER_HEALTH;
	PLAYER_ARR[0]->SetRealCharacter(blockType); // 0x023ed588
	SAVE_DATA.currentCharacter = blockType;
	
	if (!healPlayer)
		PLAYER_HEALTH = prev_health;
	
	// play the sound effect of the character saying their name
	Sound::Play(1, soundIDs[blockType], camSpacePos);
	
	Particle::System::NewSimple(0x09, pos.x, pos.y, pos.z);
	
	jiggleState = 1;
	
	return;
}

// jiggles block up then down.
void CharacterBlock::Jiggle()
{
	if (!isUnlocked || jiggleState == 0)
		return;
	
	switch (jiggleState)
	{
		case 1:
			pos.y += 0x9600_f;
			modelSolid.mat4x3.c1 *= 0x1'720_f;
			modelSolid.data.UpdateVertsUsingBones();
			
			if (pos.y > oldPos.y + 0x20202_f)
				jiggleState = 2;
			
			break;
		
		case 2:
			pos.y -= 0x6400_f;
			
			if (pos.y < oldPos.y)
				jiggleState = 3;
			
			break;
		
		case 3:
			pos.y = oldPos.y;
			canBeHit = true;
			jiggleState = 0;
			break;
	}
}

void CharacterBlock::UpdateModelTransform()
{
	if (isUnlocked)
	{
		modelSolid.mat4x3 = Matrix4x3::RotationY(ang.y);
		modelSolid.mat4x3.c3 = pos >> 3;

		DropShadowScaleXYZ(shadow, modelSolid.mat4x3, 0x85000_f, 0x150000_f, 0x85000_f, 0xc);
	}
	else
	{
		modelTrans.mat4x3 = Matrix4x3::RotationY(ang.y);
		modelTrans.mat4x3.c3 = pos >> 3;

		DropShadowScaleXYZ(shadow, modelTrans.mat4x3, 0x85000_f, 0x150000_f, 0x85000_f, 0xc);
	}
}

[[gnu::target("thumb")]]
int CharacterBlock::InitResources()
{
	blockType = ((param1 & 0xf) > 3 ? 0 : param1 & 0xf);
	needsUnlock = ((param1 >> 4 & 0xf) != 1);
	startingCharacter = ((param1 >> 8 & 0xf) == 0 ? 3 : (param1 >> 8 & 0xf) - 1);
	healPlayer = ((param1 >> 12 & 0xf) == 1);
	canBeHit = true;
	
	isUnlocked = CheckUnlock();
	currentYoshi = CheckYoshi();
	
	SetHighestUnlockedYoshi();
	
	nextYoshi = currentYoshi + 1;
	if (nextYoshi > highestUnlockedYoshi)
		nextYoshi = 0;
	
	if (SAVE_DATA.currentCharacter != 3)
		nextYoshi = currentYoshi;
	
	setYoshi = nextYoshi;
	
	if (isUnlocked)
	{
		soundIDs[0] = 27;
		soundIDs[1] = 91;
		soundIDs[2] = 155;
		soundIDs[3] = 219;
		
		BMD_File& modelF = modelFiles[0].LoadBMD();
		modelSolid.SetFile(modelF, 1, -1);
		
		BTP_File& texSeqF = texSeqFiles[0].LoadBTP();
		modelF.PrepareAnim(texSeqF);
		texSeqSolid.SetFile(texSeqF, Animation::LOOP, 0x10000_f, blockType + (blockType == 3 ? setYoshi : 0));

		KCL_File& clsnF = clsnFile.LoadKCL();
		clsn.SetFile(&clsnF, clsnNextMat, 0x190_f, ang.y, CharacterBlockCLPS::instance<>);
		
		shadow.InitCuboid();
		
		UpdateModelTransform();
		UpdateClsnPosAndRot();
	}
	else
	{
		BMD_File& modelF = modelFiles[1].LoadBMD();
		modelTrans.SetFile(modelF, 1, -1);
		
		BTP_File& texSeqF1 = texSeqFiles[1].LoadBTP();
		modelF.PrepareAnim(texSeqF1);
		texSeqTrans.SetFile(texSeqF1, Animation::NO_LOOP, 0x10000_f, blockType);
		
		BTP_File& texSeqF2 = texSeqFiles[2].LoadBTP();
		modelF.PrepareAnim(texSeqF2);
		texSeqTransMark.SetFile(texSeqF2, Animation::NO_LOOP, 0x10000_f, blockType);
		
		BCA_File& animF = animFile.LoadBCA();
		modelTrans.SetAnim(animF, Animation::LOOP, 1._f, 0);
		
		shadow.InitCuboid();
		
		UpdateModelTransform();
	}
	
	shadowMat = model.mat4x3 * Matrix4x3::IDENTITY;
	shadowMat.c3.y -= 20._f >> 3;
	
	jiggleState = 0;
	oldPos = pos;
	
	
	/*if (SAVE_DATA.currentCharacter == 3 && !hasYoshiUpdated)
	{
		ClosestPlayer()->SetRealCharacter(3);
		hasYoshiUpdated = true;
	}*/
	
	return 1;
}

int CharacterBlock::CleanupResources()
{
	clsn.Disable();
	clsnFile.Release();
	
	modelFiles[0].Release();
	modelFiles[1].Release();
	
	texSeqFiles[0].Release();
	texSeqFiles[1].Release();
	texSeqFiles[2].Release();
	
	animFile.Release();
	
	return 1;
}

[[gnu::target("thumb")]]
void CharacterBlock::SetHighestUnlockedYoshi()
{
	unsigned starsCollected = CountStars();
	if (starsCollected >= 60) // change this to the amount of stars needed for Yellow Yoshi
	{
		highestUnlockedYoshi = 3;
	}
	else if (starsCollected >= 35) // change this to the amount of stars needed for Blue Yoshi
	{
		highestUnlockedYoshi = 2;
	}
	else if (starsCollected >= 12) // change this to the amount of stars needed for Red Yoshi
	{
		highestUnlockedYoshi = 1;
	}
	else
	{
		highestUnlockedYoshi = 0;
	}
}

int CharacterBlock::Behavior()
{
	UpdateModelTransform();
	
	if (isUnlocked)
	{
		SetHighestUnlockedYoshi();
		
		if (setYoshi != nextYoshi && blockType == 3)
		{
			//cout << (3 + nextYoshi) << '\n';
			texSeqSolid.currFrame = (Fix12i)(3 + nextYoshi);
			setYoshi = nextYoshi;
		}
		
		Jiggle();
		
		if(IsClsnInRange(0_f, 0_f))
			UpdateClsnPosAndRot();
	}
	else
	{
		modelTrans.Advance();
	}
	
	hasYoshiUpdated = false;
	
	return 1;
}

int CharacterBlock::Render()
{
	if (isUnlocked)
	{
		modelSolid.Render(nullptr);
		texSeqSolid.Update(modelSolid.data);
	}
	else
	{
		modelTrans.Render(nullptr);
		texSeqTrans.Update(modelTrans.data);
		texSeqTransMark.Update(modelTrans.data);
	}
	
	return 1;
}

void CharacterBlock::OnHitFromUnderneath(Actor& attacker)
{
	JumpedUnderBlock();
	canBeHit = false;
}

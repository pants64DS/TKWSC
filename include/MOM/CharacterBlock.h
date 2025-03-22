#ifndef CHARACTER_BLOCK_INCLUDED
#define CHARACTER_BLOCK_INCLUDED

#include "SM64DS_PI.h"

struct CharacterBlock : public Platform
{	
	void UpdateModelTransform();
	void ChangeYoshiColor();
	void SetHighestUnlockedYoshi();
	unsigned CountStars();
	void Jiggle();
	void JumpedUnderBlock();
	bool CheckUnlock();
	
	virtual int CheckYoshi();
	virtual int InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	virtual void OnHitFromUnderneath(Actor& attacker) override;
	
	int jiggleState;
	bool healPlayer;
	Vector3 oldPos;
	Model modelSolid;
	ModelAnim modelTrans;
	ShadowModel shadow;
	Matrix4x3 shadowMat;
	TextureSequence texSeqSolid;
	TextureSequence texSeqTrans;
	TextureSequence texSeqTransMark;
	
	unsigned myParticle;
	uint8_t blockType;
	uint8_t startingCharacter;
	uint8_t highestUnlockedYoshi;
	bool needsUnlock;
	bool isUnlocked;
	bool canBeHit;
	bool jumpEnded;
	bool shouldChangeSpeed;
	bool shouldChangeSwimSpeed;
	int soundIDs[4];
	int setYoshi;
	
	static int currentYoshi;
	static int nextYoshi;
	static bool hasYoshiUpdated;
	
	static SpawnInfo spawnData;
	
	static SharedFilePtr modelFiles[2];
	static SharedFilePtr texSeqFiles[3];
	static SharedFilePtr clsnFile;
	static SharedFilePtr animFile;
};

#endif

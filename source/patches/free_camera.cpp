// Free camera for debugging; uncomment to enable

/*
#include <optional>
#include "SM64DS_PI.h"

static auto hasUnpaused = [prevPaused = false]() mutable -> bool
{
	return std::exchange(prevPaused, GAME_PAUSED) && !GAME_PAUSED;
};

class FreeCamera
{
	Vector3 pos;
	short angleX, angleY;
	bool fixed = false;

public:

	constexpr FreeCamera(const Camera& cam):
		pos(cam.pos),
		angleX(cam.angle.z),
		angleY(cam.angle.y)
	{}

	[[gnu::always_inline]]
	auto ToMatrix() const
	{
		return Matrix4x3::Proxy([this]<bool resMayAlias>[[gnu::always_inline]](Matrix4x3& res)
		{
			res = Matrix4x3::RotationXYZ(angleX, angleY, 0);
			res.c3 = pos >> 3;
		});
	}

	bool Update()
	{
		if (fixed) return hasUnpaused();

		Input& input = INPUT_ARR[0];

		angleX += (input.dirZ * 0.15_f).val;
		angleY -= (input.dirX * 0.15_f).val;

		const Matrix4x3 matrix = Matrix4x3::RotationXYZ(angleX, angleY, 0);
		const Fix12i speed = 500._f;

		const bool a = input.buttonsHeld & Input::A;
		const bool b = input.buttonsHeld & Input::B;
		const bool x = input.buttonsHeld & Input::X;
		const bool y = input.buttonsHeld & Input::Y;
		const bool l = input.buttonsHeld & Input::L;
		const bool r = input.buttonsHeld & Input::R;

		pos += matrix.c0 * speed * (r - l) + matrix.c2 * speed * (b - a);
		pos.y += speed * (x - y);

		// Keep the player awake
		if (Player* player = PLAYER_ARR[0])
			if (player->currState == &Player::ST_WAIT)
				player->stateTimer = 900;

		input.buttonsPressed = 0;
		input.buttonsHeld    = 0;
		input.magnitude      = 0;
		input.dirX           = 0_fs;
		input.dirZ           = 0_fs;
		input.angle          = 0;

		if (hasUnpaused()) fixed = true;

		return false;
	}
};

static std::optional<FreeCamera> freeCamera;

static SharedFilePtr modelFile;
static SharedFilePtr animFile;
static SharedFilePtr texSeqFile;

static ModelAnim*       model = nullptr;
static ShadowModel*     shadow = nullptr;
static TextureSequence* texSeq = nullptr;

alignas(ModelAnim)       static std::byte modelStorage [sizeof(*model)];
alignas(ShadowModel)     static std::byte shadowStorage[sizeof(*shadow)];
alignas(TextureSequence) static std::byte texSeqStorage[sizeof(*texSeq)];

unsigned repl_0202bbe0()
{
	if (freeCamera)
	{
		if (freeCamera->Update())
			freeCamera.reset();
	}
	else if (hasUnpaused() && CAMERA)
		freeCamera.emplace(*CAMERA);

	return 0x0209f300;
}

void repl_0200de68(Camera& cam)
{
	if (!shadow)
	{
		shadow = new (shadowStorage) ShadowModel;
		shadow->InitCylinder();
	}

	if (!model)
	{
		modelFile.FromOv0ID(0x02d7);
		animFile.FromOv0ID(0x02d8);

		BMD_File& modelData = modelFile.LoadBMD();
		char* animData = Animation::LoadFile(animFile);

		model = new (modelStorage) ModelAnim;
		model->SetFile(modelData, 1, -1);

		if (!texSeq)
		{
			texSeqFile.FromOv0ID(0x02d9);

			char* texSeqData = TextureSequence::LoadFile(texSeqFile);
			TextureSequence::Prepare(modelData, texSeqData);
			
			texSeq = new (texSeqStorage) TextureSequence;
			texSeq->SetFile(texSeqData, Animation::LOOP, 1._f, 0);
		}

		model->SetAnim(animData, Animation::LOOP, 1._f, 0);
	}

	if (freeCamera)
	{
		INV_VIEW_MATRIX_ASR_3 = freeCamera->ToMatrix();
		VIEW_MATRIX_ASR_3 = INV_VIEW_MATRIX_ASR_3.Inverse();

		model->mat4x3 = cam.camMat.Inverse();
		model->mat4x3.c0 = -model->mat4x3.c0;
		model->mat4x3.c2 = -model->mat4x3.c2;

		const Vector3 modelPos = cam.pos - model->mat4x3.c2 * 143._f - model->mat4x3.c1 * 22._f;

		model->mat4x3.c3 = modelPos >> 3;

		model->Advance();
		texSeq->Advance();
		texSeq->Update(model->data);
		model->Render(nullptr);

		static Matrix4x3 shadowMatrix;

		shadowMatrix.Linear() = Matrix3x3::Identity();
		shadowMatrix.c3 = modelPos;
		shadowMatrix.c3.y -= 56._f;
		shadowMatrix.c3 >>= 3;

		shadow->InitModel(&shadowMatrix, 70._f, 600._f, 70._f, 15);
	}
	else
		cam.View::Render();
}

template<class T>
static void Delete(T*& ptr)
{
	if (ptr)
	{
		ptr->~T();
		ptr = nullptr;
	}
}

int repl_0202cae0()
{
	freeCamera.reset();
	modelFile.Release();
	animFile.Release();
	texSeqFile.Release();

	Delete(model);
	Delete(shadow);
	Delete(texSeq);

	return 0x91c;
}
*/

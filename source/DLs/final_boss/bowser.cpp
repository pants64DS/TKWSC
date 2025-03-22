#include "SM64DS_PI.h"
#include "Actors/Bowser.h"
#include "Actors/SpikeBomb.h"

static constinit short alpha = 31;

extern "C" bool BowserBeforeBehavior(Bowser& bowser)
{
	if (!bowser.Actor::BeforeBehavior() || !PLAYER_ARR[0])
		return false;

	Player& player = *PLAYER_ARR[0];

	if (player.pos.x >= -4000._f &&
		player.pos.x <=  4000._f &&
		player.pos.y >=  -500._f &&
		player.pos.y <=  2600._f &&
		player.pos.z >= -4000._f &&
		player.pos.z <=  4600._f
	)
		ApproachLinear(alpha, 31, 1);
	else
		ApproachLinear(alpha, 0, 1);

	return alpha > 0;
}

extern "C" bool BowserBeforeRender(Bowser& bowser)
{
	alpha = std::min<int>(alpha, bowser.alpha >> 3);

	if (bowser.Actor::BeforeRender() && alpha > 0)
	{
		bowser.modelAnim.data.materials[0].SetAlpha(alpha);
		bowser.modelAnim.data.materials[1].SetAlpha(alpha);

		return true;
	}
	else return false;
}

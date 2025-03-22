#ifndef MOM_INTERFACE_INCLUDED
#define MOM_INTERFACE_INCLUDED

#include "SM64DS_PI.h"
struct LaunchStar;

struct MOM_Interface
{
	static MOM_Interface instance;

	Player::State& launchStarState;
	bool camRotationDisabled;
	void(&launchFromLaunchStar)(Player&, LaunchStar&);
};

#endif
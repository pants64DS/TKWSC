#include "MOM_Interface.h"
#include "LaunchStar.h"

[[gnu::section("MOM_Interface")]]
constinit MOM_Interface MOM_Interface::instance =
{
	.launchStarState = LaunchStar::playerState,
	.camRotationDisabled = false,
	.launchFromLaunchStar = LaunchStar::Launch
};

#include "LaunchStar.h"

void InterpCubic(Vector3& n, Vector3& a, Vector3& b, Vector3& c, Vector3& d, Fix12i currTime)
{
	Fix12i iVar5 = a.y;
	Fix12i iVar6 = a.z;
	Fix12i iVar3 = b.y;
	Fix12i iVar7 = b.z;
	Fix12i iVar1 = c.y;
	Fix12i iVar4 = c.z;
	Fix12i iVar2 = d.z;

	n.x = (b.x * 3 - a.x) + c.x * -3 + d.x;

	n.y = (iVar3 * 3 - iVar5) + iVar1 * -3 + d.y;
	n.z = (iVar7 * 3 - iVar6) + iVar4 * -3 + iVar2;
	n *= currTime;

	iVar6 = a.y;
	iVar3 = a.z;
	iVar5 = b.y;
	iVar1 = b.z;
	iVar2 = c.y;
	iVar4 = c.z;

	n.x += a.x  * 3 + b.x  * -6 + c.x  * 3;
	n.y += iVar6 * 3 + iVar5 * -6 + iVar2 * 3;
	n.z += iVar3 * 3 + iVar1 * -6 + iVar4 * 3;

	n *= currTime;

	iVar2 = a.y;
	iVar4 = b.y;
	iVar1 = b.z;
	iVar3 = a.z;

	n.x += a.x  * -3 + b.x  * 3;
	n.y += iVar2 * -3 + iVar4 * 3;
	n.z += iVar3 * -3 + iVar1 * 3;

	n *= currTime;
	n += a;
}

bool BezierPathIter::Advance()
{
	while (true)
	{
		do
		{
			Vector3 a = pathPtr.GetNode(currSplineX3);
			Vector3 b = pathPtr.GetNode(currSplineX3 + 1);
			Vector3 c = pathPtr.GetNode(currSplineX3 + 2);
			Vector3 d = pathPtr.GetNode(currSplineX3 + 3);
			Vector3 n;
			
			//Identical replacement function.
			InterpCubic(n, a, b, c, d, currTime);
			
			const Fix12i dist = metric(n, pos);
			
			if (step <= dist)
			{
				//Fix12i result = FX_Div(step, dist);
				Fix12i result = step / dist; //Proper division replacement.

				//Func_02090DD0(&pos, &n, &pos, 0x1000 - result);

				//Replacement (just linear interpolation).
				pos -= n;
				pos *= 1._f - result;
				pos += n;
				
				return true;
			}

			if (currTime >= 1._f && pathPtr->numNodes <= currSplineX3 + 6)
			{
				pos = pathPtr.GetNode(currSplineX3 + 3);
				return false;
			}
			
			currTime += tinyStep;

		}
		while (currTime < 1._f || pathPtr->numNodes <= currSplineX3 + 6);
		
		do
		{
			currTime -= 1._f;
			currSplineX3 += 3;
			
			if(currTime < 1._f) break;
		}
		while (currSplineX3 + 6 < pathPtr->numNodes);
	}
}
//PRIMORDIAL STAR IS A HOMEBREW PATHFINDING BASED ON TAUT PATHS
//I LEARNED LATER IT IS EQUIVALENT TO VISIBILITY GRAPHS
//BUT ITS PRETTY OPTIMIZED AND IM SUPER PROUD OF IT 

#pragma once
#include <sc2api/sc2_api.h>

#include "map2d.hpp"
#include "helpers.hpp"
#include "profiler.hpp"
#include "bitmap.hpp"

#include <queue>

using namespace sc2;

namespace PrimordialStar {
	
	constexpr int DISTANCENODE_DIVISIONS = 16;
	constexpr int MAX_CONN_DIST_SQRD = 65025;

	constexpr float CORNER_DISPLACEMENT_EPSILON = 0.1F;
	constexpr float MICHAEL_VISIBILE_EPSILON = 0.1F;

#define DEBUG_SPECIAL 0

	enum Cardinal {
		INVALID,
		UP, 
		UP_RT, //CORNER UP_RT MEANS THE CORNER IS TO THE DOWN AND LEFT
		RT,
		DN_RT, //CORNER DN_RT MEANS THE CORNER IS TO THE UP AND LEFT
		DN,
		DN_LT, //CORNER DN_LT MEANS THE CORNER IS TO THE UP AND RIGHT
		LT,
		UP_LT //CORNER UP_LT MEANS THE CORNER IS TO THE DOWN AND RIGHT
	};

	struct FurthestWallDistanceNode {
		float distances[DISTANCENODE_DIVISIONS];
	};

	//struct NearestWallDistanceNode {
	//	float distance;
	//};

	class AStarNode {
	public:
		//PathNode this compares to
		int pathNode;

		//distance already travelled
		float g;

		//optimistic distance to travel
		float h;

		AStarNode(int pathNode, float g, float h) : pathNode(pathNode), g(g), h(h) {

		}

		operator float() {
			return g + h;
		}
	};

	class AStarCompare {
	public:
		bool operator()(AStarNode a, AStarNode b)
		{
			return (a.g + a.h > b.g + b.h);
		}
	};

	class DijkStarNode {
	public:

		//PathNode this compares to
		int pathNode;

		//distance already travelled
		float g;

		DijkStarNode(int pathNode, float g) : pathNode(pathNode), g(g) {

		}

		operator float() {
			return g;
		}
	};

	class DijkstraCompare {
	public:
		bool operator()(DijkStarNode a, DijkStarNode b)
		{
			return (a.g > b.g);
		}
	};

	Point2D cardinalToDirection(Cardinal& cardinal) {
		switch (cardinal) {
		case(UP): {
			return { 0,1 };
		}
		case(UP_RT): {
			return { 1,1 };
		}
		case(RT): {
			return { 1,0 };
		}
		case(DN_RT): {
			return { 1,-1 };
		}
		case(DN): {
			return { 0,-1 };
		}
		case(DN_LT): {
			return { -1,-1 };
		}
		case(LT): {
			return { -1,0 };
		}
		case(UP_LT): {
			return { -1,1 };
		}
		default: {
			return { 0,0 };
		}
		}
	}

	Point2D cardinalToNormDirection(Cardinal& cardinal) {
		switch (cardinal) {
		case(UP): {
			return { 0,1 };
		}
		case(UP_RT): {
			return { 0.7071067812F,0.7071067812F };
		}
		case(RT): {
			return { 1,0 };
		}
		case(DN_RT): {
			return { 0.7071067812F,-0.7071067812F };
		}
		case(DN): {
			return { 0,-1 };
		}
		case(DN_LT): {
			return { -0.7071067812F,-0.7071067812F };
		}
		case(LT): {
			return { -1,0 };
		}
		case(UP_LT): {
			return { -0.7071067812F,0.7071067812F };
		}
		default: {
			return { 0,0 };
		}
		}
	}

	int numberPreExit = 0;
	int numberSpecial = 0;
	float maxDiff = 0;
	float minDiff = MAX_CONN_DIST_SQRD;

	struct Connected {
		int pathNodeID;
		float distance;
		bool special;

		bool operator==(const Connected& conn) const {
			return pathNodeID == conn.pathNodeID;
		}
	};

	//bool operator==(const Connected&  conn1, const Connected&  conn2) {
	//	return conn1.pathNodeID == conn2.pathNodeID && conn1.distance == conn2.distance;
	//}

	class PathNode {
	private:
		Point2D pos;
	public:

		Cardinal wall;
		int id;
		std::vector<Connected> connected;

		PathNode(Point2D pos, Cardinal wall, Agent* agent);

		~PathNode();

		void updatePos(Point2D p) {
			pos = p;
		}

		Point2D rawPos() {
			return pos;
		}

		Point2D position(float radius) {
			if (wall == INVALID) {
				return pos;
			}
			return pos - cardinalToNormDirection(wall) * radius;
		}
	};

	//vector of every path node
	std::vector<PathNode*> allPathNodes;

	//map of the furthest wall at every given point
	map2d<FurthestWallDistanceNode>* furthestWallGrid;

	////map of the closest wall at every given point
	//map2d<NearestWallDistanceNode>* nearestWallGrid;

	int maxConnections = 0;
	float maxDistanceConnectionSquared = 0.0F;

	//DDA https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)
	bool checkLinearPath(Point2D start, Point2D end) {
		float dx = end.x - start.x;
		float dy = end.y - start.y;
		float step = 0;

		if (abs(dx) >= abs(dy)) {
			step = abs(dx);
		}
		else {
			step = abs(dy);
		}

		dx /= step;
		dy /= step;

		Point2D operating = start;
		for (int i = 0; i < step; i++) {
			if (!Aux::isPathable(int(operating.x), int(operating.y))) {
				return false;
			}
			operating += {dx, dy};
		}
		return true;
	}

	inline float rnd(float in) {
		return (float)(int)(in + 0.1F);
	}

	Point2D getWall(Point2D origin, Point2D dir, int maxSteps = 255) {
		//var delta = Vector2.Normalize(direction - origin);
		Point2D delta = Aux::normalize(dir);

		float dxdy = delta.y == 0 ? 0 : delta.x / delta.y;
		float dydx = delta.x == 0 ? 0 : delta.y / delta.x;

		// If delta X is 0, we set the rayLength to a big one so that the Y ray is always chosen (we're moving straight up or straight down)
		float rayLengthWhenMovingInX = FLT_MAX;
		if (delta.x != 0) {
			rayLengthWhenMovingInX = sqrt(1 + dydx * dydx);
		}

		// If delta Y is 0, we set the rayLength to a big one so that the X ray is always chosen (we're moving straight left or straight right)
		float rayLengthWhenMovingInY = FLT_MAX;
		if (delta.y != 0) {
			rayLengthWhenMovingInY = sqrt(1 + dxdy * dxdy);
		}

		// Edge case, if deltaX is 0, stepXDistance can be 0, making the first ray 0, thus it's going to be picked
		// We want to avoid that so we set it to 1
		float stepX = 1;
		float stepXDistance = 1.0F;
		if (delta.x > 0) {
			// Moving right
			stepX = 1;
			stepXDistance = floor(origin.x + 1) - origin.x;
		}
		else if (delta.x < 0) {
			// Moving left
			stepX = -1;
			stepXDistance = origin.x - floor(origin.x);
		}

		// Edge case, if deltaY is 0, stepYDistance can be 0, making the first ray 0, thus it's going to be picked
		// We want to avoid that so we set it to 1
		float stepY = 1;
		float stepYDistance = 1.0F;
		if (delta.y > 0) {
			// Moving up
			stepY = 1;
			stepYDistance = floor(origin.y + 1) - origin.y;
		}
		else if (delta.y < 0) {
			// Moving down
			stepY = -1;
			stepYDistance = origin.y - floor(origin.y);
		}

		Point2D lastIntersection = origin;
		Point2D currentCell = Aux::P2D(Aux::P2DI(origin));
		float xRayLength = stepXDistance * rayLengthWhenMovingInX;
		float yRayLength = stepYDistance * rayLengthWhenMovingInY;

		while (Aux::isPathable((int)(currentCell.x), (int)(currentCell.y))) {
			if (xRayLength < yRayLength) {
				// Step in X, reduce Y ray
				yRayLength -= xRayLength;

				// Move to the cell on the left or right
				currentCell.x += stepX;
				lastIntersection += delta * xRayLength;

				// Due to float imprecision, we get some rounding errors
				// We can round X to the closest int to eliminate the error because we know we stepped in X (int, float)
				lastIntersection.x = rnd(lastIntersection.x);

				// Reset X ray
				xRayLength = rayLengthWhenMovingInX;
			}
			else if (yRayLength < xRayLength) {
				// Step in Y, reduce X ray
				xRayLength -= yRayLength;

				// Move to the cell on the bottom or top
				currentCell.y += stepY;
				lastIntersection += delta * yRayLength;

				// Due to float imprecision, we get some rounding errors
				// We can round Y to the closest int to eliminate the error because we know we stepped in Y (float, int)
				lastIntersection.y = rnd(lastIntersection.y);

				// Reset Y ray
				yRayLength = rayLengthWhenMovingInY;
			}
			else {
				// Both rays are the same, means we landed exactly on a corner
				currentCell.x += stepX; // Move to the left/right
				currentCell.y += stepY; // And up/down
				lastIntersection += delta * yRayLength; // xRayLength and yRayLength are the same, doesn't matter which one we pick

				// Due to float imprecision, we get some rounding errors
				// We can round to the closest int to eliminate the error because we know we are a at corner (int, int)
				lastIntersection.x = rnd(lastIntersection.x);
				lastIntersection.y = rnd(lastIntersection.y);

				// Reset all rays
				xRayLength = rayLengthWhenMovingInX;
				yRayLength = rayLengthWhenMovingInY;
			}
		}
		return lastIntersection;
	}

	float checkWallDistance(Point2D origin, Point2D dir, int maxSteps = 255) {
		return Distance2D(origin, getWall(origin, dir, maxSteps));
	}

	float checkWallDistanceSquared(Point2D origin, Point2D dir, int maxSteps = 255) {
		return DistanceSquared2D(origin, getWall(origin, dir, maxSteps));
	}

	float loadMaxDistanceGrid(Agent* const agent) {
		float maximum = 0;
		for (int i = 0; i < Aux::mapWidth_cache; i++) {
			for (int j = 0; j < Aux::mapHeight_cache; j++) {
				bool center = Aux::isPathable(i, j);
				if (center) {
					float max = 0;
					constexpr int perQuad = 8;
					constexpr int angleChecks = perQuad * DISTANCENODE_DIVISIONS;
					int steps = 255;
					

					FurthestWallDistanceNode maximums;
					//float maxes[perQuad] = { 0 };
					for (int theta = 0; theta < angleChecks; theta++) {
						float angle = 2 * MY_PI * theta / angleChecks;
						Point2D dir = { cos(angle), sin(angle) };
						float dist = checkWallDistance(Point2D{ i + 0.5F, j + 0.5F }, dir, steps);
						if (dist > max) {
							max = dist;
							//steps = dist + 15;
						}
						if (dist > maximum) {
							maximum = dist;
						}

						if (theta % perQuad == (perQuad - 1)) {
							maximums.distances[theta / perQuad] = max;
							max = 0;
						}
					}

					imRef(furthestWallGrid, i, j) = maximums;
				}
				else {
					imRef(furthestWallGrid, i, j) = { 0.0 };
				}
			}
		}
		return maximum;
	}

	void calculateNewConnection(PathNode* p, Agent* agent) {
		Point2D pos = p->rawPos();
		int intX = (int)(pos.x);
		int intY = (int)(pos.y);
		if (intX < 0 || intX >= furthestWallGrid->width() || intY < 0 || intY >= furthestWallGrid->height()) {
			return;
		}
		for (int i = 0; i < allPathNodes.size(); i++) {
			if (i == p->id) {
				continue;
			}
			PrimordialStar::PathNode* node = PrimordialStar::allPathNodes[i];
			Point2D testPos = node->rawPos();

			if (p->wall != INVALID && node->wall != INVALID &&
				(((p->wall == UP_LT || node->wall == DN_RT) && testPos.x > pos.x && testPos.y < pos.y) ||
					((p->wall == UP_RT || node->wall == DN_LT) && testPos.x < pos.x && testPos.y < pos.y) ||
					((p->wall == DN_LT || node->wall == UP_RT) && testPos.x > pos.x && testPos.y > pos.y) ||
					((p->wall == DN_RT || node->wall == UP_LT) && testPos.x < pos.x && testPos.y > pos.y))) {
				continue;
			}
			float distSqrd = DistanceSquared2D(pos, testPos);
			float dist = sqrt(distSqrd);

			float innerMax = 255; //imRef(maxDistanceGrid, int(pos.x), int(pos.y)).distance;
			
			Point2D direction = testPos - pos;

			float angle = atan2f(direction.y, direction.x);
			float angleSections = MY_2PI / DISTANCENODE_DIVISIONS;

			if (angle < 0) angle += MY_2PI;

			int index1 = (int)(angle * DISTANCENODE_DIVISIONS / MY_2PI);
			int index2 = (int)(Aux::floatmod(angle + MY_PI, MY_2PI) * DISTANCENODE_DIVISIONS / MY_2PI);

			innerMax = std::max(imRef(furthestWallGrid, intX, intY).distances[index1], imRef(furthestWallGrid, intX, intY).distances[index2]);

			//if (testPos.x > pos.x && testPos.y >= pos.y) {
			//	innerMax = imRef(furthestWallGrid, intX, intY).distancePP;
			//}
			//else if (testPos.x <= pos.x && testPos.y > pos.y) {
			//	innerMax = imRef(furthestWallGrid, intX, intY).distanceNP;
			//}
			//else if (testPos.x < pos.x && testPos.y <= pos.y) {
			//	innerMax = imRef(furthestWallGrid, intX, intY).distanceNN;
			//}
			//else if (testPos.x >= pos.x && testPos.y < pos.y) {
			//	innerMax = imRef(furthestWallGrid, intX, intY).distancePN;
			//}

			bool special = false;

			if (distSqrd > ((innerMax + 2) * (innerMax + 2))) {
				numberPreExit++;
#if DEBUG_SPECIAL
				special = true; 
#else
				continue;
#endif
			}
			if (checkLinearPath(pos, testPos)) {

				if (distSqrd > MAX_CONN_DIST_SQRD) {
					continue;
				}
#if DEBUG_SPECIAL
				if (special) {
					numberSpecial++;
					float diff = sqrt(distSqrd) - innerMax;
					if (diff < minDiff){
						minDiff = diff;
					}
					if (diff > maxDiff) {
						maxDiff = diff;
					}
				}
#endif
				if (maxDistanceConnectionSquared < distSqrd) {
					maxDistanceConnectionSquared = distSqrd;
				}
				int m = std::max((int)(p->connected.size()), (int)(node->connected.size()));
				if (maxConnections < m) {
					maxConnections = m;
				}
				p->connected.push_back( { i, dist, special } );
				node->connected.push_back( { p->id, dist, special } );
			}
		}
	}

	void breakAllConnections(PathNode* p) {
		for (int i = 0; i < p->connected.size(); i++) {
			if (p->connected[i].pathNodeID >= PrimordialStar::allPathNodes.size()) {
				continue;
			}
			PrimordialStar::PathNode* node = PrimordialStar::allPathNodes[p->connected[i].pathNodeID];
			for (int c = 0; c < node->connected.size(); c++) {
				if (node->connected[c].pathNodeID == p->id) {
					node->connected.erase(node->connected.begin() + c);
				}
			}
		}
	}

	PathNode::PathNode(Point2D pos, Cardinal wall, Agent* agent) : pos(pos), wall(wall) {
		id = (int)(allPathNodes.size());
		allPathNodes.push_back(this);
		calculateNewConnection(this, agent);
	}

	PathNode::~PathNode() {
		breakAllConnections(this);
	}

	bool check_UP_RT(int i, int j) {
		bool up = Aux::isPathable(i, j + 1);
		bool up_rt = Aux::isPathable(i + 1, j + 1);
		bool rt = Aux::isPathable(i + 1, j);

		//extra trimming conditions
		bool up_lt = Aux::isPathable(i - 1, j + 1);
		bool dn_rt = Aux::isPathable(i + 1, j - 1);

		return up && up_rt && rt && (up_lt || dn_rt);
	}

	bool check_DN_RT(int i, int j) {
		bool rt = Aux::isPathable(i + 1, j);
		bool dn_rt = Aux::isPathable(i + 1, j - 1);
		bool dn = Aux::isPathable(i, j - 1);

		//extra trimming conditions
		bool up_rt = Aux::isPathable(i + 1, j + 1);
		bool dn_lt = Aux::isPathable(i - 1, j - 1);

		return dn && dn_rt && rt && (up_rt || dn_lt);
	}

	bool check_DN_LT(int i, int j) {
		bool dn = Aux::isPathable(i, j - 1);
		bool dn_lt = Aux::isPathable(i - 1, j - 1);
		bool lt = Aux::isPathable(i - 1, j);

		//extra trimming conditions
		bool dn_rt = Aux::isPathable(i + 1, j - 1);
		bool up_lt = Aux::isPathable(i - 1, j + 1);

		return dn && dn_lt && lt && (dn_rt || up_lt);
	}

	bool check_UP_LT(int i, int j) {
		bool lt = Aux::isPathable(i - 1, j);
		bool up_lt = Aux::isPathable(i - 1, j + 1);
		bool up = Aux::isPathable(i, j + 1);

		//extra trimming conditions
		bool dn_lt = Aux::isPathable(i - 1, j - 1);
		bool up_rt = Aux::isPathable(i + 1, j + 1);

		return up && up_lt && lt && (dn_lt || up_rt);
	}

	void generatePathNodes(Agent* agent) {
		for (int i = 1; i < Aux::mapWidth_cache - 1; i++) {
			for (int j = 1; j < Aux::mapHeight_cache - 1; j++) {
				bool center = Aux::isPathable(i, j);
				if (!center) {

					if (check_UP_RT(i, j) && ((Aux::isPathable(i - 1, j + 1) || !check_UP_RT(i - 1, j + 1)) || (Aux::isPathable(i + 1, j - 1) || !check_UP_RT(i + 1, j - 1)))) {
						new PathNode(Point2D{ (float)(i + 1 + CORNER_DISPLACEMENT_EPSILON), (float)(j + 1 + CORNER_DISPLACEMENT_EPSILON) }, DN_LT, agent);
					}
					if (check_DN_RT(i, j) && ((Aux::isPathable(i - 1, j - 1) || !check_DN_RT(i - 1, j - 1)) || (Aux::isPathable(i + 1, j + 1) || !check_DN_RT(i + 1, j + 1)))) {
						new PathNode(Point2D{ (float)(i + 1 + CORNER_DISPLACEMENT_EPSILON), (float)(j - CORNER_DISPLACEMENT_EPSILON) }, UP_LT, agent);
					}
					if (check_DN_LT(i, j) && ((Aux::isPathable(i - 1, j + 1) || !check_DN_LT(i - 1, j + 1)) || (Aux::isPathable(i + 1, j - 1) || !check_DN_LT(i + 1, j - 1)))) {
						new PathNode(Point2D{ (float)(i - CORNER_DISPLACEMENT_EPSILON), (float)(j - CORNER_DISPLACEMENT_EPSILON) }, UP_RT, agent);
					}
					if (check_UP_LT(i, j) && ((Aux::isPathable(i - 1, j - 1) || !check_UP_LT(i - 1, j - 1)) || (Aux::isPathable(i + 1, j + 1) || !check_UP_LT(i + 1, j + 1)))) {
						new PathNode(Point2D{ (float)(i - CORNER_DISPLACEMENT_EPSILON), (float)(j + 1 + CORNER_DISPLACEMENT_EPSILON) }, DN_RT, agent);
					}
				}
			}
		}

		printf("MAX CONNECTIONS OF A NODE: %d\t MAX DISTANCE OF A CONNECTION: %.2f\n", maxConnections, sqrt(maxDistanceConnectionSquared));
	}

	PathNode* startNode;

	void setupTerminalNode(PathNode* terminalNode, Point2D selfPos, Point2D otherEndPos, Agent* const agent) {
		if (terminalNode->connected.size() == 0) {
			Point2D loc = allPathNodes[0]->rawPos();
			float mindist = DistanceSquared2D(loc, selfPos);
			for (int i = 1; i < allPathNodes.size() - 2; i++) {
				float dist = DistanceSquared2D(allPathNodes[i]->rawPos(), selfPos);
				if (dist < mindist || (dist == mindist && (DistanceSquared2D(loc, otherEndPos) > DistanceSquared2D(allPathNodes[i]->rawPos(), otherEndPos)))) {
					mindist = dist;
					loc = allPathNodes[i]->rawPos();
				}
			}
			terminalNode->updatePos(loc);
			calculateNewConnection(terminalNode, agent);
		}
	}

	std::vector<Point2D> getPathAStar(Point2D start, Point2D end, float radius, Agent* const agent) {
		Profiler profiler("getPathAStar");

		if (checkWallDistanceSquared(start, (start - end)) >= DistanceSquared2D(start, end)) {
			std::vector<Point2D> p;
			p.push_back(start);
			p.push_back(end);
			profiler.midLog("getPathAStar.quickEnd");
			return p;
		}

		startNode = new PathNode(start, INVALID, agent);
		profiler.midLog("getPathAStar.startN");

		PathNode* operatingNode = startNode;
		PathNode* endNode = new PathNode(end, INVALID, agent);
		profiler.midLog("getPathAStar.endN");

		bool* visited = new bool[allPathNodes.size()];
		memset(visited, 0, allPathNodes.size() * sizeof(bool));

		std::vector<Point2D> points;
		std::map<int, int> backpath;
		profiler.midLog("getPath.init");

		setupTerminalNode(startNode, start, end, agent);
		setupTerminalNode(endNode, end, start, agent);

		profiler.midLog("getPath.correction");

		if (std::find_if(startNode->connected.begin(), startNode->connected.end(),
			[&node = endNode->id]
			(const Connected& m) -> bool { return node == m.pathNodeID; }
		) != startNode->connected.end()) {
			profiler.subScope();
			points.push_back(startNode->rawPos());
			points.push_back(endNode->rawPos());
			profiler.midLog("getPath.skip");
		}
		else {
			profiler.subScope();
			std::priority_queue<AStarNode, std::vector<AStarNode>, AStarCompare> starNodes;
			starNodes.push(AStarNode(operatingNode->id, 0, Distance2D(start, end)));
			visited[operatingNode->id] = true;
			bool found = false;
			for (int cycles = 0; cycles < 10000; cycles++) {
				if (starNodes.size() == 0) {
					break;
				}
				AStarNode star = starNodes.top();
				starNodes.pop();
				operatingNode = allPathNodes[star.pathNode];
				Point2D currentPos = operatingNode->position(radius);
				for (int i = 0; i < operatingNode->connected.size(); i++) {
					int subNodeID = operatingNode->connected[i].pathNodeID;
					if (visited[subNodeID]) {
						continue;
					}
					Point2D nextPos = allPathNodes[subNodeID]->position(radius);
					backpath[subNodeID] = operatingNode->id;
					starNodes.push(AStarNode(subNodeID, star.g + Distance2D(currentPos, nextPos), Distance2D(nextPos, end)));
					visited[subNodeID] = true;

					//DebugText(agent, strprintf("%.1f,%.1f", star.g + Distance2D(currentPos, nextPos), Distance2D(nextPos, end)), AP3D(nextPos));

					if (operatingNode->connected[i].pathNodeID == endNode->id) {
						found = true;
						break;
					}
				}
				if (found) {
					break;
				}
			}
			profiler.midLog("getPath.source");
			operatingNode = endNode;
			points.push_back(allPathNodes[operatingNode->id]->rawPos());
			for (int i = 0; i < starNodes.size(); i++) {
				operatingNode = allPathNodes[backpath[operatingNode->id]];
				points.insert(points.begin(), allPathNodes[operatingNode->id]->position(radius));
				if (operatingNode->id == startNode->id) {
					break;
				}
			}
			profiler.midLog("getPath.backtrack");
		}

		delete[] visited;
		allPathNodes.pop_back();
		allPathNodes.pop_back();

		delete startNode;
		startNode = nullptr;
		delete endNode;
		startNode = nullptr;

		//printf("SOMETHING WENT WRONG\n");
		profiler.midLog("getPath.end");
		return points;
	}

	void doDijkstra(std::priority_queue<DijkStarNode, std::vector<DijkStarNode>, DijkstraCompare> &Q, std::vector<float> &dist, std::vector<int> &parent, std::vector<Cardinal> &incoming) {
		dist.reserve(allPathNodes.size());
		parent.reserve(allPathNodes.size());
		incoming.reserve(allPathNodes.size());
		for (int i = 0; i < allPathNodes.size(); i++) {
			dist.push_back(FLT_MAX);
			parent.push_back(-1);
			incoming.push_back(INVALID);
		}

		Q.push(DijkStarNode(startNode->id, 0));
		dist[startNode->id] = 0;

		while (Q.size() > 0) {
			DijkStarNode u = Q.top(); Q.pop();

			PathNode* operatingNode = allPathNodes[u.pathNode];
			Point2D rawPosOp = operatingNode->rawPos();
			for (int i = 0; i < operatingNode->connected.size(); i++) {
				PathNode* adjacentNode = allPathNodes[operatingNode->connected[i].pathNodeID];
				Point2D rawPosAdj = adjacentNode->rawPos();

				float weight = operatingNode->connected[i].distance;//Distance2D(rawPosOp, rawPosAdj);

				if (dist[adjacentNode->id] > dist[u.pathNode] + weight) {

					parent[adjacentNode->id] = u.pathNode;
					dist[adjacentNode->id] = dist[u.pathNode] + weight;

					Q.push(DijkStarNode(adjacentNode->id, dist[adjacentNode->id]));
				}
			}
		}
	}

	std::vector<Point2D> getPathDijkstra(Point2D start, Point2D end, float radius, Agent* agent) {
		Profiler profiler("getDijkstra");

		if(checkLinearPath(start, end)){
			std::vector<Point2D> p;
			p.push_back(start);
			p.push_back(end);
			profiler.midLog("getDijkstra.quickEnd");
			return p;
		}

		startNode = new PathNode(start, INVALID, agent);

		PathNode* endNode = new PathNode(end, INVALID, agent);

		std::vector<Point2D> points;
		profiler.midLog("getDijkstra.init");

		setupTerminalNode(startNode, start, end, agent);
		setupTerminalNode(endNode, end, start, agent);

		profiler.midLog("getDijkstra.correction");

		if (startNode->connected.size() == 0 || endNode->connected.size() == 0) {

		}
		else {
			std::priority_queue<DijkStarNode, std::vector<DijkStarNode>, DijkstraCompare> Q;

			std::vector<float> dist;
			std::vector<int> parent;
			std::vector<Cardinal> incoming;
			
			doDijkstra(Q, dist, parent, incoming);

			int node = endNode->id;
			for (int cycles = 0; cycles < 10000; cycles++) {
				if (node == -1) {
					points.clear();
					break;
				}
				points.insert(points.begin(), allPathNodes[node]->position(radius));
				if (node == startNode->id) {
					break;
				}
				node = parent[node];
			}
		}

		allPathNodes.pop_back();
		allPathNodes.pop_back();

		delete startNode;
		startNode = nullptr;
		delete endNode;
		endNode = nullptr;

		return points;
	}

	std::vector<std::vector<Point2D>> getPathDijkstra(Point2D start, std::vector<Point2D> ends, float radius, Agent* agent) {
		Profiler profiler("getDijkstraBatch");

		std::vector<std::vector<Point2D>> points;

		std::vector<PathNode*> endNodes;
		endNodes.reserve(ends.size());
		points.reserve(ends.size());
		profiler.midLog("getDijkstraBatch.reserve");
		for (Point2D end : ends) {
			PathNode* singleEnd = new PathNode(end, INVALID, agent);
			setupTerminalNode(singleEnd, end, start, agent);
			endNodes.push_back(singleEnd);
		}

		profiler.midLog("getDijkstraBatch.initEnds");
		startNode = new PathNode(start, INVALID, agent);
		
		profiler.midLog("getDijkstraBatch.initStart");

		setupTerminalNode(startNode, start, start, agent);

		//for (PathNode* singleEnd : endNodes) {
		//	setupTerminalNode(singleEnd, singleEnd->rawPos(), start, agent);
		//}

		profiler.midLog("getDijkstraBatch.correction");

		if (startNode->connected.size() == 0) {

		}
		else {
			std::priority_queue<DijkStarNode, std::vector<DijkStarNode>, DijkstraCompare> Q;

			std::vector<float> dist;
			std::vector<int> parent;
			std::vector<Cardinal> incoming;

			doDijkstra(Q, dist, parent, incoming);

			profiler.midLog("getDijkstraBatch.flow");

			for (PathNode* singleEnd : endNodes) {
				std::vector<Point2D> p;
				int node = singleEnd->id;
				for (int cycles = 0; cycles < 10000; cycles++) {
					if (node == -1) {
						p.clear();
						break;
					}
					p.insert(p.begin(), allPathNodes[node]->position(radius));
					if (node == startNode->id) {
						break;
					}
					node = parent[node];
				}
				points.push_back(p);
			}
			profiler.midLog("getDijkstraBatch.backtrack");
		}
		profiler.subScope();

		allPathNodes.pop_back();
		delete startNode;
		startNode = nullptr;
		for (int i = 0; i < endNodes.size(); i++) {
			allPathNodes.pop_back();
			delete endNodes[i];
			endNodes[i] = nullptr;
		}
		profiler.midLog("getDijkstraBatch.delete");
		return points;
	}

	float getPathLength(std::vector<Point2D> path) {
		if (path.size() == 0) return 0.0F;
		float travelled = 0;
		for (int i = 0; i < path.size() - 1; i++) {
			travelled += Distance2D(path[i], path[i + 1]);
		}
		return travelled;
	}

	float getPathLengthAStar(Point2D start, Point2D end, float radius, Agent* agent) {
		return getPathLength(getPathAStar(start, end, radius, agent));
	}

	float getPathLengthDijkstra(Point2D start, Point2D end, float radius, Agent* agent) {
		return getPathLength(getPathDijkstra(start, end, radius, agent));
	}

	Point2D distanceAlongPath(std::vector<Point2D> path, float distance) {
		if (path.size() == 0) return { 0,0 };
		float travelled = 0;
		for (int i = 0; i < path.size() - 1; i++) {
			float dist = Distance2D(path[i], path[i + 1]);
			if ((travelled + dist) < distance) {
				travelled += dist;
			}
			else {
				Point2D dir = Aux::normalize((path[i + 1] - path[i]));
				return path[i] + (dir * (distance - travelled));
			}
		}
		return { 0,0 };
	}

	std::vector<Point2D> stepPointsAlongPath(std::vector<Point2D> path, float distance) {
		if (path.size() == 0) return std::vector<Point2D>();
		int siz = int(400 / distance);
		std::vector<Point2D> midpath;
		for (int i = 1; i < siz; i++) {
			Point2D ne = distanceAlongPath(path, distance * i);

			if (ne != Point2D{ 0, 0 }) {
				midpath.push_back(ne);
			}
			else {
				break;
			}
		}
		return midpath;
	}

	void pathVerification(Agent* const agent) {
#define NUM_PTS_RT 80
		std::vector<Point2D> pts;
		pts.reserve(NUM_PTS_RT);

		for (int asd = 0; asd < NUM_PTS_RT; asd++) {
			pts.push_back(Aux::getRandomPathable(agent));
		}

		std::vector<float> differenceInDistance;
		differenceInDistance.reserve(NUM_PTS_RT * NUM_PTS_RT);

		timeus startTime;
		timeus endTime;
		long long dt;
		float mean;
		float min;
		float max;

		//startTime = std::chrono::steady_clock::now();
		//for (int a = 0; a < NUM_PTS_RT; a++) {
		//	Point2D from = pts[a];
		//	DebugSphere(agent, Aux::P3D(agent, from), 0.25, { 61,102,220 });
		//	for (int b = 0; b < NUM_PTS_RT; b++) {
		//		Point2D to = pts[b];
		//		auto path = getPathDijkstra(from, to, 0, agent);
		//		float dist = getPathLength(path);
		//		float sc2dist = agent->Query()->PathingDistance(from, to);
		//		float diff = dist - sc2dist;
		//		//if (diff < 0) {
		//		//    printf("S{%.1f,%.1f} E{%.1f,%.1f} D[%.1f]\n", from.x, from.y, to.x, to.y, diff);
		//		//}
		//		if (diff > 2 || diff < -2) {
		//			Color c = Aux::randomColor();
		//			if (diff < 0) {
		//				c.r = 0;
		//			}
		//			else if (diff > 0) {
		//				c.g = 0;
		//			}
		//			float z = std::rand() * 2.0F / RAND_MAX;
		//			if (path.size() > 0) {
		//				for (int i = 0; i < path.size() - 1; i++) {
		//					DebugLine(agent, Aux::P3D(agent, path[i]) + Point3D{ 0,0,1.5F + z }, Aux::P3D(agent, path[i + 1]) + Point3D{ 0,0,1.5F + z }, c);
		//				}
		//			}
		//			else {
		//				DebugLine(agent, Aux::P3D(agent, from) + Point3D{ 0,0,1.5F + z }, Aux::P3D(agent, to) + Point3D{ 0,0,1.5F + z }, c);
		//			}
		//		}
		//		differenceInDistance.push_back(diff);
		//	}
		//}

		//endTime = std::chrono::steady_clock::now();
		//dt = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
		//mean = 0;
		//min = 400;
		//max = 0;
		//for (float f : differenceInDistance) {
		//	if (f < min) {
		//		min = f;
		//	}
		//	if (f > max) {
		//		max = f;
		//	}
		//	mean += f;
		//	//printf("%.1f\n", f);
		//}
		//mean /= differenceInDistance.size();
		//printf("PATH VERIFICATION SINGLE [%d^2] %.1fs MEAN:%.1f MIN:%.1f MAX:%.1f\n", NUM_PTS_RT, dt / 1000000.0, mean, min, max);
		//SendDebug(agent);

		startTime = std::chrono::steady_clock::now();
		for (int a = 0; a < NUM_PTS_RT; a++) {
			Point2D from = pts[a];
			DebugSphere(agent, Aux::P3D(agent, from), 0.25, { 61,102,220 });
			std::vector<std::vector<Point2D>> paths = getPathDijkstra(from, pts, 0, agent);
			for (int b = 0; b < paths.size(); b++) {
				auto path = paths[b];
				float dist = getPathLength(path);
				float sc2dist = agent->Query()->PathingDistance(from, pts[b]);
				float diff = dist - sc2dist;
				//if (diff < 0) {
				//    printf("S{%.1f,%.1f} E{%.1f,%.1f} D[%.1f]\n", from.x, from.y, to.x, to.y, diff);
				//}
				if (diff > 2 || diff < -2) {
					Color c = Aux::randomColor();
					if (diff < 0) {
						c.r = 0;
					}
					else if (diff > 0) {
						c.g = 0;
					}
					float z = std::rand() * 2.0F / RAND_MAX;
					if (path.size() > 0) {
						for (int i = 0; i < path.size() - 1; i++) {
							DebugLine(agent, Aux::P3D(agent, path[i]) + Point3D{ 0,0,1.5F + z }, Aux::P3D(agent, path[i + 1]) + Point3D{ 0,0,1.5F + z }, c);
						}
					}
					else {
						DebugLine(agent, Aux::P3D(agent, from) + Point3D{ 0,0,1.5F + z }, Aux::P3D(agent, pts[b]) + Point3D{ 0,0,1.5F + z }, c);
					}

				}
				differenceInDistance.push_back(diff);
			}
		}

		endTime = std::chrono::steady_clock::now();
		dt = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
		mean = 0;
		min = 400;
		max = 0;
		for (float f : differenceInDistance) {
			if (f < min) {
				min = f;
			}
			if (f > max) {
				max = f;
			}
			mean += f;
			//printf("%.1f\n", f);
		}
		mean /= differenceInDistance.size();
		printf("PATH VERIFICATION BATCH [%d^2] %.1fs MEAN:%.1f MIN:%.1f MAX:%.1f\n", NUM_PTS_RT, dt / 1000000.0, mean, min, max);
		SendDebug(agent);
	}

	void load(Agent* const agent) {
		furthestWallGrid = new map2d<FurthestWallDistanceNode>(Aux::mapWidth_cache, Aux::mapHeight_cache, false);
		//nearestWallGrid = new map2d<NearestWallDistanceNode>(Aux::mapWidth_cache, Aux::mapHeight_cache, false);

		timeus timecheck = std::chrono::steady_clock::now();
		float maximalMax = loadMaxDistanceGrid(agent);
		printf("Generating Max Distance Grid took %.3fms\n", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timecheck).count() / 1000.0);

		timecheck = std::chrono::steady_clock::now();
		generatePathNodes(agent);
		printf("Generating %lu Path Nodes took %.3fms ", allPathNodes.size(), std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timecheck).count() / 1000.0);

		int numConnections = 0;

		for (PrimordialStar::PathNode* node : PrimordialStar::allPathNodes) {
			numConnections += (int)(node->connected.size());
		}

		
#if DEBUG_SPECIAL
		printf("with %d connections and %d quick bypasses and %d specials [%.1f, %.1f]\n", numConnections, numberPreExit, numberSpecial, minDiff, maxDiff);
#else
		printf("with %d connections and %d quick bypasses\n", numConnections, numberPreExit);
#endif

#if 0
		for (int i = 0; i < PrimordialStar::allPathNodes.size(); i++) {
			PrimordialStar::PathNode* node = PrimordialStar::allPathNodes[i];
			if (Distance2D(node->rawPos(), agent->Observation()->GetCameraPos()) > 300) {
				continue;
			}
			DebugSphere(agent, Aux::P3D(agent, node->rawPos()), 0.5, { 250,50,100 });
			for (int c = 0; c < node->connected.size(); c++) {
				PrimordialStar::PathNode* node2 = PrimordialStar::allPathNodes[node->connected[c].pathNodeID];
				if (node->connected[c].special) {
					DebugLine(agent, Aux::P3D(agent, node->rawPos()) + Point3D{ 0,0,1 }, Aux::P3D(agent, node2->rawPos()) + Point3D{ 0,0,1 }, Colors::Red);
				}
				else {
					DebugLine(agent, Aux::P3D(agent, node->rawPos()) + Point3D{ 0,0,1 }, Aux::P3D(agent, node2->rawPos()) + Point3D{ 0,0,1 }, Colors::Blue);
				}
				
			}
		}
		SendDebug(agent);

		pathVerification(agent);
		printf("PATH VERIFICATION ENDED\n");
#endif
		for (int a = 0; a < DISTANCENODE_DIVISIONS; a++) {
			saveBitmap(strprintf("furthestWall%d.bmp", a), Aux::mapWidth_cache, Aux::mapHeight_cache,
				[maximalMax, a](int i, int j) {return uint8_t(imRef(furthestWallGrid, i, j).distances[a] / maximalMax * 255.0);},
				[maximalMax, a](int i, int j) {return uint8_t(imRef(furthestWallGrid, i, j).distances[a] / maximalMax * 255.0);},
				[maximalMax, a](int i, int j) {return uint8_t(imRef(furthestWallGrid, i, j).distances[a] / maximalMax * 255.0);});
		}

	}
}

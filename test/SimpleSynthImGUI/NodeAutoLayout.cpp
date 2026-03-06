#pragma once
#include "NodeAutoLayout.h"
#include "imgui.h"
using namespace yzrilyzr_collection;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;

void NodeAutoLayout::start(CurrentProjectContext * ctx, int mode){
	// 初始化盒子
	nodeIdToBoxMap.clear();
	nodeLines.clear();
	//填入盒子大小
	for(ProjectObject * obj : ctx->objects){
		int nodeId=reinterpret_cast<int>(obj);
		ImVec2 pos=ImNodes::GetNodeGridSpacePos(nodeId);
		ImVec2 dimen=ImNodes::GetNodeDimensions(nodeId);
		nodeIdToBoxMap[nodeId]=AutoLayoutBox(obj, pos, dimen, 1, obj->lockLayout);
	}
	//添加一个接受输出（即只有一个输入）的节点盒，这是固定位置和大小的
	ImVec2 pos=ImNodes::GetNodeGridSpacePos(OUTPUT_NODE_ID);
	ImVec2 dimen=ImNodes::GetNodeDimensions(OUTPUT_NODE_ID);
	nodeIdToBoxMap[OUTPUT_NODE_ID]=AutoLayoutBox(nullptr, pos, dimen, (double)1e99, true);
	nodeIdToBoxMap[OUTPUT_NODE_ID].pos=pos; // 固定位置

	int lineHeight=30;
	int titleBarHeight=50;
	for(LinkLine & link : ctx->links){
		AutoLayoutLine line{nullptr, nullptr, 0};
		//查找左右连接
		for(auto & pair : nodeIdToBoxMap){
			int nodeId=pair.first;
			auto & box=pair.second;
			if(nodeId == link.started_at_node_id){
				line.left=&nodeIdToBoxMap[nodeId];
				line.leftPosOffset=titleBarHeight + lineHeight / 2;
			}
			if(nodeId == link.ended_at_node_id){
				line.right=&nodeIdToBoxMap[nodeId];
				int paramIndex=1;
				//计算参数位置垂直偏移量，接受输出的不需要计算
				if(line.right->obj != nullptr){
					for(auto & param : line.right->obj->paramRegPtr->RegisteredParams){
						int attrId=reinterpret_cast<int>(param.value);
						if(attrId == link.ended_at_attribute_id){
							break;
						}
						paramIndex++;
					}
				}
				line.rightPosOffset=titleBarHeight + lineHeight * paramIndex + lineHeight / 2;
			}
		}
		if(line.left == nullptr || line.right == nullptr)continue;

		nodeLines.emplace_back(line);
	}
	state=mode;
	autoLayoutDT.getAndSetDeltaTime();
	startTimer.getAndSetDeltaTime();
	springTimer.getAndSetDeltaTime();

	// 初始化碰撞记录
	collisionRecord.clear();
	lastCollisionTime=0;
}

void NodeAutoLayout::computeSpring(float xOffset){
	//执行牵拉（弹簧力）
	for(auto & line : nodeLines){
		auto & boxLeft=*line.left;
		auto & boxRight=*line.right;

		// 如果两个盒子都被锁定，跳过
		if((boxLeft.lock && boxLeft.obj != nullptr) && (boxRight.lock && boxRight.obj != nullptr)) continue;

		//计算牵拉点
		ImVec2 leftPoint{boxLeft.pos.x + boxLeft.size.x + xOffset, boxLeft.pos.y + line.leftPosOffset};
		ImVec2 rightPoint{boxRight.pos.x - xOffset, boxRight.pos.y + line.rightPosOffset};
		/*if(boxLeft.obj != nullptr){
			ImVec2 leftNodePos=ImNodes::GetNodeScreenSpacePos(reinterpret_cast<int>(boxLeft.obj));
			ImVec2 pointPos(leftNodePos.x + boxLeft.size.x, leftNodePos.y + line.leftPosOffset);
			ImGui::GetForegroundDrawList()->AddCircleFilled(pointPos, 10, 0xff0000ff);
		}
		if(boxRight.obj != nullptr){
			ImVec2 rightNodePos=ImNodes::GetNodeScreenSpacePos(reinterpret_cast<int>(boxRight.obj));
			ImVec2 pointPos(rightNodePos.x, rightNodePos.y + line.rightPosOffset);
			ImGui::GetForegroundDrawList()->AddCircleFilled(pointPos, 10, 0xff0000ff);
		}*/
		ImVec2 direction=ImVec2(rightPoint.x - leftPoint.x, rightPoint.y - leftPoint.y);
		double distance=vec2distance(leftPoint, rightPoint);

		if(distance > 0.1){
			direction.x/=distance;
			direction.y/=distance;

			// 弹簧力
			double forceMagnitude=distance * SPRING_K;

			ImVec2 forceOnLeft=ImVec2(direction.x * forceMagnitude, direction.y * forceMagnitude);
			ImVec2 forceOnRight=ImVec2(-direction.x * forceMagnitude, -direction.y * forceMagnitude);

			// 应用力时检查锁定状态
			if(boxLeft.mass > 0 && !boxLeft.lock){
				boxLeft.acc.x+=forceOnLeft.x / boxLeft.mass;
				boxLeft.acc.y+=forceOnLeft.y / boxLeft.mass;
			}

			if(boxRight.mass > 0 && boxRight.obj != nullptr && !boxRight.lock){
				boxRight.acc.x+=forceOnRight.x / boxRight.mass;
				boxRight.acc.y+=forceOnRight.y / boxRight.mass;
			}
		}
	}
}

void NodeAutoLayout::computeRepulsion(){
	// 计算盒子间的排斥力 - 使用中心连线方向，确保无重叠
	std::vector<AutoLayoutBox *> boxes;
	for(auto & pair : nodeIdToBoxMap){
		boxes.push_back(&pair.second);
	}

	const double EPSILON=0.001f;
	const double ANGLE_THRESHOLD=0.707f;  // 45度角的cos/sin值，用于区分区域

	for(size_t j=0; j < boxes.size(); j++){
		for(size_t k=j + 1; k < boxes.size(); k++){
			AutoLayoutBox * boxA=boxes[j];
			AutoLayoutBox * boxB=boxes[k];

			// 如果两个都是固定节点或都被锁定，跳过
			if((boxA->obj == nullptr && boxB->obj == nullptr) ||
			   (boxA->lock && boxB->lock)) continue;

			// 更精确的AABB重叠检测
			bool overlapX=(boxA->pos.x < boxB->pos.x + boxB->size.x + IDEAL_DISTANCE) &&
				(boxB->pos.x < boxA->pos.x + boxA->size.x + IDEAL_DISTANCE);
			bool overlapY=(boxA->pos.y < boxB->pos.y + boxB->size.y + IDEAL_DISTANCE) &&
				(boxB->pos.y < boxA->pos.y + boxA->size.y + IDEAL_DISTANCE);

			// 如果不重叠，跳过
			if(!overlapX || !overlapY){
				continue;
			}

			// 以下代码只有在重叠时才会执行
			// 计算两个盒子的中心点
			ImVec2 centerA(boxA->pos.x + boxA->size.x * 0.5f, boxA->pos.y + boxA->size.y * 0.5f);
			ImVec2 centerB(boxB->pos.x + boxB->size.x * 0.5f, boxB->pos.y + boxB->size.y * 0.5f);

			// 计算中心连线向量和距离
			ImVec2 delta=ImVec2(centerA.x - centerB.x, centerA.y - centerB.y);
			double distance=Math::hypot(delta.x, delta.y);

			// 如果中心点重合，使用默认方向
			ImVec2 direction;
			if(distance < EPSILON){
				// 随机选择一个方向，避免所有重合节点都朝同一方向运动
				direction=ImVec2(1, 0);
				distance=1.0;
			} else{
				direction.x=delta.x / distance;
				direction.y=delta.y / distance;
			}

			// 计算中心连线的角度（使用方向向量的绝对值来判断区域）
			float absDirX=Math::abs(direction.x);
			float absDirY=Math::abs(direction.y);

			// 计算穿透深度（使用原始方法）
			double projA=Math::abs((boxA->size.x * 0.5) * direction.x) +
				Math::abs((boxA->size.y * 0.5) * direction.y);
			double projB=Math::abs((boxB->size.x * 0.5) * direction.x) +
				Math::abs((boxB->size.y * 0.5) * direction.y);
			double desiredDistance=projA + projB + IDEAL_DISTANCE;
			double penetrationDepth=desiredDistance - distance;
			if(penetrationDepth < 0) penetrationDepth=0;

			// 根据角度区域决定排斥力的方向
			ImVec2 forceDirection;

			if(absDirY > absDirX){
				// 垂直区域（|dy| > |dx|）：上下推挤
				// 使用垂直方向为主，但保留少量水平分量以保持自然运动
				forceDirection.x=direction.x * 0.3f;  // 保留30%的水平分量
				forceDirection.y=direction.y > 0?1.0f:-1.0f;  // 纯垂直方向

				// 归一化
				float forceDirLen=Math::hypot(forceDirection.x, forceDirection.y);
				if(forceDirLen > EPSILON){
					forceDirection.x/=forceDirLen;
					forceDirection.y/=forceDirLen;
				}
			} else{
				// 水平区域（|dx| >= |dy|）：左右推挤
				// 使用水平方向为主，但保留少量垂直分量以保持自然运动
				forceDirection.x=direction.x > 0?1.0f:-1.0f;  // 纯水平方向
				forceDirection.y=direction.y * 0.3f;  // 保留30%的垂直分量

				// 归一化
				float forceDirLen=Math::hypot(forceDirection.x, forceDirection.y);
				if(forceDirLen > EPSILON){
					forceDirection.x/=forceDirLen;
					forceDirection.y/=forceDirLen;
				}
			}

			// 计算排斥力大小
			double forceMagnitude=REPULSION_FORCE * penetrationDepth * penetrationDepth * 2.0;

			// 力的大小限制，避免过冲
			const double MAX_FORCE=800.0;
			const double MIN_FORCE=5.0;
			forceMagnitude=Util::clamp(forceMagnitude, MIN_FORCE, MAX_FORCE);

			// 计算最终的力向量（使用调整后的方向）
			ImVec2 forceOnA=ImVec2(forceDirection.x * forceMagnitude, forceDirection.y * forceMagnitude);
			ImVec2 forceOnB=ImVec2(-forceDirection.x * forceMagnitude, -forceDirection.y * forceMagnitude);

			// 应用加速度时检查锁定状态
			if(boxA->mass > 0 && boxA->obj != nullptr && !boxA->lock){
				boxA->acc.x+=forceOnA.x / boxA->mass;
				boxA->acc.y+=forceOnA.y / boxA->mass;
			}

			if(boxB->mass > 0 && boxB->obj != nullptr && !boxB->lock){
				boxB->acc.x+=forceOnB.x / boxB->mass;
				boxB->acc.y+=forceOnB.y / boxB->mass;
			}


		}
	}
}

void NodeAutoLayout::doPhysic(){
	// 力学系统参数
	double TIME_STEP=1;//autoLayoutDT.getAndSetDeltaTime() / 1000.0f;//秒
	// 执行物理模拟
	for(auto & pair : nodeIdToBoxMap){
		auto & box=pair.second;
		if(box.obj == nullptr) continue;

		// 应用加速度更新速度
		box.vel.x+=box.acc.x * TIME_STEP;
		box.vel.y+=box.acc.y * TIME_STEP;

		// 应用阻尼
		box.vel.x*=(1.0f - DAMPER);
		box.vel.y*=(1.0f - DAMPER);

		// 速度限制
		double speed=Math::hypot(box.vel.x, box.vel.y);
		if(speed > MAX_SPEED){
			box.vel.x=(box.vel.x / speed) * MAX_SPEED;
			box.vel.y=(box.vel.y / speed) * MAX_SPEED;
		}

		// 更新位置
		box.pos.x+=box.vel.x * TIME_STEP;
		box.pos.y+=box.vel.y * TIME_STEP;
	}

	//检查盒子能量
	totalEnergy=0;
	for(auto & pair : nodeIdToBoxMap){
		auto & box=pair.second;
		if(box.obj != nullptr){
			totalEnergy+=0.5f * box.mass * (box.vel.x * box.vel.x + box.vel.y * box.vel.y);
		}
	}

	// 应用布局结果
	for(auto & pair : nodeIdToBoxMap){
		auto & box=pair.second;
		if(box.obj == nullptr) continue;
		ProjectObject * obj=box.obj;
		int nodeId=reinterpret_cast<int>(obj);
		obj->windowPos=box.pos;
		ImNodes::SetNodeGridSpacePos(nodeId, obj->windowPos);
	}

	// 清理旧的碰撞记录（超过500ms的）
	float currentTime=autoLayoutDT.getDeltaTime();
	for(auto it=collisionRecord.begin(); it != collisionRecord.end();){
		if(currentTime - it->second > 500.0f){ // 500ms
			it=collisionRecord.erase(it);
		} else{
			++it;
		}
	}
}

void NodeAutoLayout::doLayout(){
	// 重置加速度
	for(auto & pair : nodeIdToBoxMap){
		pair.second.acc=ImVec2(0, 0);
	}
	//连线牵拉（输入输出点）吸引+节点盒重叠排斥力	
	if(state == SPRING){
		//初始状态仅用弹簧快速布局
		DAMPER=0.1;
		MAX_SPEED=100;
		SPRING_K=0.5;
		REPULSION_FORCE=0.0;
		computeSpring(30);
		if(totalEnergy < 10){
			if(springTimer.isDeltaTimeAndSet(1000)){
				state=SPRING_REPULSION;
			}
		} else{
			springTimer.setDeltaTime();
		}
	} else if(state == SPRING_REPULSION){
		DAMPER=0.99;
		MAX_SPEED=3;
		SPRING_K=1;
		REPULSION_FORCE=0.2;
		computeSpring(20);
		computeRepulsion();
		if(totalEnergy < 100){
			if(springTimer.isDeltaTimeAndSet(1000)){
				state=FINISH;
			}
		} else{
			springTimer.setDeltaTime();
		}
	}
	doPhysic();
}

bool NodeAutoLayout::finished(){
	if(state == FINISH)return true;
	if(startTimer.isDeltaTime(10000)){
		//限制10秒内执行
		state=FINISH;
		return true;
	}
	return state == FINISH;
}
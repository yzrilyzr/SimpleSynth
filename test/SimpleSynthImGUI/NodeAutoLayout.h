#pragma once
#include "util/Random.h"
#include "util/DT.h"
#include "lang/Math.h"
#include "imgui.h"
#include "imnodes.h"
#include "SimpleSynthProject.h"


struct AutoLayoutBox{
	ProjectObject * obj;
	ImVec2 pos;
	ImVec2 size;
	ImVec2 vel;//速度
	ImVec2 acc;//加速度
	double mass;//质量
	bool lock;

	AutoLayoutBox(){}
	AutoLayoutBox(ProjectObject * obj, ImVec2 pos, ImVec2 size, double mass, bool lock) :
		obj(obj), size(size), pos(pos), vel(ImVec2(0, 0)), acc(ImVec2(0, 0)), mass(mass), lock(lock){}
};

struct AutoLayoutLine{
	AutoLayoutBox * left;//线的左边，连接输出端口
	AutoLayoutBox * right;//线的右边，接受输出的端口
	float leftPosOffset;//输出端口坐标是固定的(x+w,y+固定值)
	float rightPosOffset;//接受输出端口的位置偏移，端口实际坐标=(x,y+rightPosOffset）
};

inline double vec2distance(const ImVec2 & a, const ImVec2 & b){
	return yzrilyzr_lang::Math::hypot(a.x - b.x, a.y - b.y);
}

class NodeAutoLayout{
	private:
	CurrentProjectContext * ctx=nullptr;
	yzrilyzr_util::DT autoLayoutDT;
	yzrilyzr_util::DT startTimer;
	yzrilyzr_util::DT springTimer;
	double totalEnergy=0;
	std::vector<AutoLayoutLine> nodeLines;
	std::unordered_map<int, AutoLayoutBox> nodeIdToBoxMap;
	std::unordered_map<uint64_t, double> collisionRecord; // 记录碰撞时间（毫秒）
	float lastCollisionTime=0; // 最后一次碰撞的时间
	int state=FINISH;
	void computeSpring(float xOffset);
	void computeRepulsion();
	void doPhysic();
	public:
	static const int FINISH=0;
	static const int SPRING=1;
	static const int SPRING_REPULSION=2;

	double DAMPER=0.5;//盒子运动摩擦阻尼
	double MAX_SPEED=1000;//
	double SPRING_K=10;//弹簧劲度系数
	double REPULSION_FORCE=20;//斥力系数 - 增大以更强力地推开重叠盒子
	double IDEAL_DISTANCE=15.0f;//理想间距 - 减小，让盒子可以更靠近但不重叠


	NodeAutoLayout(){}
	void start(CurrentProjectContext * ctx, int mode=SPRING);
	void doLayout();
	bool finished();
};
#include <komo_wrapper.h>

#include "komo_tree_groundings.h"

#include <Kin/switch.h>
#include <Kin/TM_angVel.h>
#include <Kin/TM_default.h>
#include <Kin/TM_gravity.h>
#include <Kin/TM_InsideBox.h>
#include <Kin/TM_AboveBox.h>
#include <Kin/TM_qLimits.h>

#include <observation_tasks.h>
#include <axis_alignment.h>

using namespace rai;
using W = mp::KomoWrapper;

double shapeSize(const KinematicWorld& K, const char* name, uint i=2);

void groundTreeInit( const mp::TreeBuilder& tb, KOMO_ext* komo, int verbose )
{

}

void groundTreePickUp(const mp::Interval& it, const mp::TreeBuilder& tb, const std::vector<std::string>& facts, KOMO_ext* komo, int verbose)
{
  groundTreeUnStack(it, tb, facts, komo, verbose);
}

void groundTreeUnStack(const mp::Interval& it, const mp::TreeBuilder& tb, const std::vector<std::string>& facts, KOMO_ext* komo, int verbose)
{
  // grasp one side (can be decided here heuristically)

  const auto& eff = "franka_hand";
  const auto& object = facts[0].c_str();

  // approach
  mp::Interval before{{it.time.to - 0.3, it.time.to - 0.3}, it.edge};
  W(komo).addObjective( before, tb, new TargetPosition( eff, object, ARR( 0.0, 0.0, 0.1 ) ), OT_sos, NoArr, 1e2, 0 ); // coming from above

  mp::Interval just_before{{it.time.to - 0.2, it.time.to}, it.edge};
  W(komo).addObjective( just_before, tb, new AxisOrthogonal( eff, ARR( 0.0, 1.0, 0.0 ), ARR( 0, 0, 1.0 ) ), OT_sos, NoArr, 1e2, 0 ); // the Y axis (lon axis of the gripper with finger) is orthogonal to Z
  W(komo).addObjective( just_before, tb, new BoxAxisAligned( eff, ARR( 0.0, 1.0, 0 ), object, ARR( -1.0, 0.0, 0 ) ), OT_sos, NoArr, 1e2, 0 ); // pick orthogonal to the sides

  // switch
  mp::Interval st{{it.time.to, it.time.to}, it.edge};

  W(komo).addSwitch(st, tb, new KinematicSwitch(SW_effJoint, JT_quatBall, eff, object, komo->world, SWInit_zero, 0, {}));

  // after
  mp::Interval future{{it.time.to, it.time.to + 1.0}, it.edge}; // fix for at least one step
  mp::Interval end{{it.time.to, it.time.to}, it.edge};

  W(komo).addObjective(future, tb, new TM_ZeroQVel(komo->world, object), OT_eq, NoArr, 3e1, 1, +1, -1); // prevent rotation once kin switch applied
  if(komo->k_order > 1)
  {
    W(komo).addObjective(end, tb, new TM_LinAngVel(komo->world, object), OT_eq, NoArr, 1e1, 2, +0, +1); // prevent object from jumping
    mp::Interval just_after{{it.time.to, it.time.to + 0.2}, it.edge};
    W(komo).addObjective( just_after, tb, new ZeroVelocity( object ), OT_eq, NoArr, 1e2, 1 ); // force the object not to move when starting to pick (mainly to force it not to go under the table)
  }

  if(verbose > 0)
  {
    std::cout << "from: " << it.time.from << "(" << it.edge.from << ")" << " -> " << it.time.to << "(" << it.edge.to << ")" <<  " : unstack " << facts[0] << " from " << facts[1] << std::endl;
  }
}

void groundTreePutDown(const mp::Interval& it, const mp::TreeBuilder& tb, const std::vector<std::string>& facts, KOMO_ext* komo, int verbose)
{
  const auto& object = facts[0].c_str();
  const auto& place = facts[1].c_str();

  return;

  // approach  
  //mp::Interval before{{it.time.to - 0.3, it.time.to - 0.3}, it.edge};
  //W(komo).addObjective( before, tb, new TargetPosition( object, place, ARR( 0.0, 0.0, 0.2 ) ), OT_sos, NoArr, 1e2, 0 ); // coming from above

  mp::Interval end{{it.time.to, it.time.to}, it.edge};
  W(komo).addObjective(end, tb, new TM_AboveBox(komo->world, object, place), OT_ineq, NoArr, 1e1, 0);

  // switch
  mp::Interval st{{it.time.to, it.time.to}, it.edge};
  Transformation rel{0};
  rel.pos.set(0,0, .5*(shapeSize(komo->world, place) + shapeSize(komo->world, object)));
  W(komo).addSwitch(st, tb, new KinematicSwitch(SW_effJoint, JT_transXYPhi, place, object, komo->world, SWInit_zero, 0, rel));

  // after (stay stable)
  mp::Interval future{{it.time.to, it.time.to + 1.0}, it.edge}; // how to improve it? ground until the end sounds inefficient!
  W(komo).addObjective(future, tb, new TM_ZeroQVel(komo->world, object), OT_eq, NoArr, 3e1, 1, +1, -1);
  if(komo->k_order > 1)
  {
    W(komo).addObjective(end, tb, new TM_LinAngVel(komo->world, object), OT_eq, NoArr, 1e1, 2, +0, +1);
  }

  if(verbose > 0)
  {
    std::cout << "from: " << it.time.from << "(" << it.edge.from << ")" << " -> " << it.time.to << "(" << it.edge.to << ")" << " : put down " << facts[0] << " at " << facts[1] << std::endl;
  }
}

void groundTreeCheck(const mp::Interval& it, const mp::TreeBuilder& tb, const std::vector<std::string>& facts, KOMO_ext* komo, int verbose)
{
  std::map<std::string, arr> sideToPivot{
    {"side_0", ARR( 0.05, 0.0, 0.02 )},
    {"side_1", ARR( 0.00, 0.05, 0.02 )},
    {"side_2", ARR( -0.05, 0.00, 0.02 )},
    {"side_3", ARR( 0.00, -0.05, 0.02 )},
    {"side_4", ARR( 0.00, 0.00, 0.05 )},
    {"side_5", ARR( 0.00, 0.00, -0.05 )}
  };

//  if(
//     //facts[1] == "side_0" ||
//     //facts[1] == "side_1" ||
//     //facts[1] == "side_2" ||
//     //facts[1] == "side_3" ||
//     //facts[1] == "side_4" ||
//     //facts[1] == "side_5"
//     )
//  {
//    return;
//  }

  mp::Interval end{{it.time.to - 0.2, it.time.to}, it.edge};
  W(komo).addObjective(end, tb, new ActiveGetSight( "franka_hand", facts[0].c_str(), sideToPivot[facts[1].c_str()], ARR( 0, 0, -1 ), 0.2 ), OT_eq, NoArr, 1e2, 0 );

  if(verbose > 0)
  {
    std::cout << "from: " << it.time.from << "(" << it.edge.from << ")" << " -> " << it.time.to << "(" << it.edge.to << ")" << " : check " << facts[0] << " " << facts[1] << std::endl;
  }
}

void groundTreeStack(const mp::Interval& it, const mp::TreeBuilder& tb, const std::vector<std::string>& facts, KOMO_ext* komo, int verbose)
{
  groundTreePutDown(it, tb, facts, komo, verbose);
}
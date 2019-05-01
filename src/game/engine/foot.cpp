/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Pathfinding layer of game unit class hierachy.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "foot.h"
#include "building.h"
#include "coord.h"
#include "gamedebug.h"
#include "globals.h"
#include "iomap.h"
#include "session.h"
#include "team.h"
#include <algorithm>
#include <cstdlib>

// Size in bits of each element of the path flags array.
#define PATH_FLAG_BITSIZE 32

#ifndef CHRONOSHIFT_STANDALONE
static cell_t &StartLocation = Make_Global<cell_t>(0x0065D7AE);
static cell_t &DestLocation = Make_Global<cell_t>(0x0065D7AC);
static unsigned *MainOverlap = Make_Pointer<unsigned>(0x0065BFAC);
static unsigned *LeftOverlap = Make_Pointer<unsigned>(0x0065C7AC);
static unsigned *RightOverlap = Make_Pointer<unsigned>(0x0065CFAC);
static int &PathCount = Make_Global<int>(0x006680EC);
#else
static cell_t StartLocation;
static cell_t DestLocation;
static unsigned MainOverlap[512]; // Is this perhaps some map size math?
static unsigned LeftOverlap[512]; // Is this perhaps some map size math?
static unsigned RightOverlap[512]; // Is this perhaps some map size math?
static int PathCount;
#endif

FootClass::FootClass(RTTIType type, int id, HousesType house) :
    TechnoClass(type, id, house),
    m_Bit1_1(false),
    m_Initiated(false),
    m_Bit1_4(false),
    m_ToLook(false),
    m_Deploying(false),
    m_Firing(false),
    m_Rotating(false),
    m_Moving(false),
    m_Bit2_1(false),
    m_InFormation(false),
    m_Bit2_4(false),
    m_ToScatter(false),
    m_Bit2_16(false),
    m_Speed(0),
    m_SpeedMult("1.0"),
    m_FormXCoefficient(INT32_MIN),
    m_FormYCoefficient(INT32_MIN),
    m_NavCom(0),
    m_SuspendedNavCom(0),
    m_Team(nullptr),
    m_field_113(TEAM_NUMBER_NONE),
    m_field_114(0),
    m_field_124((MoveType)1), // TODO: Confirm MoveTypes.
    m_PathDelay(),
    m_field_12E(10),
    m_BaseDefenseDelay(),
    m_TeamSpeed(SPEED_FOOT),
    m_TeamMaxSpeed(MPH_MIN),
    m_HeadTo(0)
{
    memset(&m_NavList, 0, sizeof(m_NavList));
    memset(&m_Paths, FACING_NONE, sizeof(m_Paths));
}

FootClass::FootClass(const FootClass &that) :
    TechnoClass(that),
    m_Bit1_1(that.m_Bit1_1),
    m_Initiated(that.m_Initiated),
    m_Bit1_4(that.m_Bit1_4),
    m_ToLook(that.m_ToLook),
    m_Deploying(that.m_Deploying),
    m_Firing(that.m_Firing),
    m_Rotating(that.m_Rotating),
    m_Moving(that.m_Moving),
    m_Bit2_1(that.m_Bit2_1),
    m_InFormation(that.m_InFormation),
    m_Bit2_4(that.m_Bit2_4),
    m_ToScatter(that.m_ToScatter),
    m_Bit2_16(that.m_Bit2_16),
    m_Speed(that.m_Speed),
    m_SpeedMult(that.m_SpeedMult),
    m_FormXCoefficient(that.m_FormXCoefficient),
    m_FormYCoefficient(that.m_FormYCoefficient),
    m_NavCom(that.m_NavCom),
    m_SuspendedNavCom(that.m_SuspendedNavCom),
    m_Team(that.m_Team),
    m_field_113(that.m_field_113),
    m_field_114(that.m_field_114),
    m_field_124(that.m_field_124),
    m_PathDelay(that.m_PathDelay),
    m_field_12E(that.m_field_12E),
    m_BaseDefenseDelay(that.m_BaseDefenseDelay),
    m_TeamSpeed(that.m_TeamSpeed),
    m_TeamMaxSpeed(that.m_TeamMaxSpeed),
    m_HeadTo(that.m_HeadTo)
{
    memcpy(&m_NavList, that.m_NavList, sizeof(m_NavList));
    memcpy(&m_Paths, that.m_Paths, sizeof(m_Paths));
}

FootClass::FootClass(const NoInitClass &noinit) : TechnoClass(noinit), m_PathDelay(noinit), m_BaseDefenseDelay(noinit) {}

FootClass::~FootClass()
{
    if (GameActive) {
        m_Team->Remove(this);
    }
}

MoveType FootClass::Can_Enter_Cell(cell_t cellnum, FacingType facing) const
{
    return MOVE_OK;
}

void FootClass::AI()
{
    TechnoClass::AI();
}

/**
 * Can we demolish this object? (can sell check?)
 *
 * 0x004C3328
 */
BOOL FootClass::Can_Demolish() const
{
    switch (What_Am_I()) {
        case RTTI_VESSEL: // Fall through
        case RTTI_UNIT: // Fall through
        case RTTI_AIRCRAFT:
            // Handle if we are heading to a building and that building is the
            // repair depot.
            if (Radio != nullptr && Radio->What_Am_I() == RTTI_BUILDING) {
                if (reinterpret_cast<BuildingClass *>(Radio)->What_Type() == BUILDING_FIX) {
                    if (Distance(Center_Coord(), Radio->Target_Coord()) < 128) {
                        return true;
                    }
                }
            }

        default: // All fall through if return not reached above.
            break;
    }

    return TechnoClass::Can_Demolish();
}

/**
 * Determine this units Y sort?
 *
 * 0x004C154C
 */
coord_t FootClass::Sort_Y() const
{
    if (m_Bit2_1) {
        return Get_Coord() + 0x1000000;
    }

    if (Radio != nullptr && Radio->What_Am_I() == RTTI_UNIT && m_Tethered) {
        return Get_Coord() + 0x1000000;
    }

    return Get_Coord() + 0x300000;
}

/**
 * Unlimbo this object.
 *
 * 0x004C1B6C
 */
BOOL FootClass::Unlimbo(coord_t coord, DirType dir)
{
    if (TechnoClass::Unlimbo(coord, dir)) {
        Revealed(m_OwnerHouse);
        m_Paths[0] = FACING_NONE;

        return true;
    }

    return false;
}

void FootClass::Detach(target_t target, int a2) {}

void FootClass::Detach_All(int a1) {}

BOOL FootClass::Mark(MarkType mark)
{
    return 0;
}

void FootClass::Active_Click_With(ActionType action, ObjectClass *object) {}

void FootClass::Active_Click_With(ActionType action, cell_t cellnum) {}

DamageResultType FootClass::Take_Damage(int &damage, int a2, WarheadType warhead, TechnoClass *object, BOOL a5)
{
    return DamageResultType();
}

void FootClass::Per_Cell_Process(PCPType pcp) {}

RadioMessageType FootClass::Receive_Message(RadioClass *radio, RadioMessageType message, target_t &target)
{
    return RadioMessageType();
}

void FootClass::Sell_Back(int a1) {}

void FootClass::Code_Pointers() {}

void FootClass::Decode_Pointers() {}

int FootClass::Mission_Attack()
{
    return 0;
}

int FootClass::Mission_Capture()
{
    return 0;
}

int FootClass::Mission_Guard()
{
    return 0;
}

int FootClass::Mission_Guard_Area()
{
    return 0;
}

int FootClass::Mission_Hunt()
{
    return 0;
}

int FootClass::Mission_Move()
{
    return 0;
}

int FootClass::Mission_Retreat()
{
    return 0;
}

int FootClass::Mission_Enter()
{
    return 0;
}

void FootClass::Override_Mission(MissionType mission, int target1, int target2) {}

BOOL FootClass::Restore_Mission()
{
    return 0;
}

void FootClass::Stun() {}

void FootClass::Death_Announcement(TechnoClass *killer) const
{
#ifndef CHRONOSHIFT_STANDALONE
    void (*func)(const FootClass *, TechnoClass *) =
        reinterpret_cast<void (*)(const FootClass *, TechnoClass *)>(0x004C3150);
    func(this, killer);
#else
    DEBUG_ASSERT_PRINT(false, "Unimplemented function '%s' called!\n", __FUNCTION__);
#endif
}

target_t FootClass::Greatest_Threat(ThreatType threat)
{
    return target_t();
}

void FootClass::Assign_Destination(target_t dest)
{
    m_field_124 = (MoveType)1; // TODO: Confirm MoveTypes.
    m_NavCom = dest;
}

coord_t FootClass::Likely_Coord()
{
    return (m_HeadTo ? m_HeadTo : Target_Coord());
}

BOOL FootClass::Start_Driver(coord_t &dest)
{
#ifndef CHRONOSHIFT_STANDALONE
    BOOL (*func)(FootClass *, coord_t &) = reinterpret_cast<BOOL (*)(FootClass *, coord_t &)>(0x004C14C8);
    return func(this, dest);
#else
    Stop_Driver();
    if (dest) {
        m_HeadTo = dest;
        m_Moving = true;
        if (Map[Coord_To_Cell(dest)].Goodie_Check(this)) {
            return true;
        }
        if (m_IsActive) {
            m_HeadTo = 0;
            m_Moving = false;
        }
    }
    return false;
#endif
}

BOOL FootClass::Stop_Driver()
{
    if (m_HeadTo) {
        m_HeadTo = 0;
        Set_Speed(0);
        m_Moving = false;
        return true;
    }
    return false;
}

BOOL FootClass::Offload_Ore_Bail()
{
    // empty in RA, but UnitClass overrides this.
    return false;
}

void FootClass::Approach_Target()
{
#ifndef CHRONOSHIFT_STANDALONE
    void (*func)(FootClass *) = reinterpret_cast<void (*)(FootClass *)>(0x005808FC);
    func(this);
#else
#endif
}

void FootClass::Fixup_Path(PathType *path)
{
    // Empty
}

/**
 * Constructs a path if possible.
 *
 * 0x004BF77C
 */
PathType *FootClass::Find_Path(cell_t dest, FacingType *buffer, int length, MoveType move)
{
    // This is Script_Unit_Pathfinder in OpenDUNE which returns the struct as
    // an object instead of using a static instance. Might be useful to study
    // for rewriting this without the gotos.
    static PathType _path;

    // If the buffer pointer is nullptr, we can't proceed so return nullptr.
    if (buffer == nullptr) {
        return nullptr;
    }

    int threat;
    int risk;
    int threat_state = 0;
    ++PathCount;

    // Are we part of a team and should that team avoid threats?
    if (!m_Team.Is_Valid() || !m_Team->Should_Avoid_Threats()) {
        threat = -1;
        risk = -1;
    } else {
        if (!m_Team.Is_Valid()) {
            risk = Risk();
        } else {
            risk = m_Team->Field35();
        }

        threat = 0;
    }

    // Set up initial state of the path
    cell_t current_cell = Coord_To_Cell(Get_Coord());

    _path.StartCell = StartLocation = current_cell;
    _path.Score = 0;
    _path.Length = 0;
    _path.Moves = buffer;
    _path.Overlap = MainOverlap;
    _path.PreviousCell = -1;
    _path.UnravelCheckpoint = -1;
    _path.Moves[0] = FACING_NONE;
    --length;
    DestLocation = dest;
#ifndef CHRONOSHIFT_STANDALONE
    memset(MainOverlap, 0, 512 * sizeof(unsigned));
#else
    memset(MainOverlap, 0, sizeof(MainOverlap));
#endif
    _path.Overlap[(uint16_t)current_cell / 32] |= 1 << ((uint16_t)current_cell % 32);

    while (_path.Length < length) {
        if (current_cell == dest) {
            break;
        }

        FacingType direction = Direction_To_Facing(Cell_Direction8(current_cell, dest));
        cell_t adj_cell = Cell_Get_Adjacent(current_cell, direction);
        int cell_score = Passable_Cell(adj_cell, direction, threat, move);

        if (cell_score) { // Great, we have a direct move, do the next round.
            Register_Cell(&_path, adj_cell, direction, cell_score, move);
            current_cell = adj_cell;
        } else { // Oops, we bumped into something, find a way around.
            FacingType right_moves[304];
            PathType right_path;
            int right_score = 0;
            FacingType left_moves[304];
            PathType left_path;
            int left_score = 0;
            PathType *chosen_path = nullptr;

            if (adj_cell == dest) { // Close enough.
                break;
            }

            int i = 0;

            while (i < 5) {
                FacingType next_dir = Direction_To_Facing(Cell_Direction8(adj_cell, dest));
                adj_cell = Cell_Get_Adjacent(adj_cell, next_dir);

                // If we still don't have passable, see if we are close.
                if (Passable_Cell(adj_cell, FACING_NONE, threat, move) == 0) {
                    // If we are close, adjust the threat and try again.
                    if (adj_cell == dest) {
                        if (threat == -1) {
                            // Set current a break to finish pathfinding.
                            current_cell = adj_cell;

                            break;
                        }

                        switch (threat_state++) {
                            case 0:
                                threat = risk / 2;
                                break;
                            case 1:
                                threat += risk;
                                break;
                            case 2:
                                threat = -1;
                                break;
                            default:
                                break;
                        }

                        break;
                    }
                }

                memcpy(&left_path, &_path, sizeof(left_path));
                left_path.Moves = left_moves;
                left_path.Overlap = LeftOverlap;
                memcpy(left_moves, _path.Moves, _path.Length);
                memcpy(_path.Overlap, left_path.Overlap, sizeof(LeftOverlap));

                left_score = Follow_Edge(current_cell,
                    adj_cell,
                    &left_path,
                    FACING_EDGE_LEFT,
                    direction,
                    threat,
                    threat_state,
                    sizeof(left_moves) - 2,
                    move);

                memcpy(&right_path, &_path, sizeof(right_path));
                right_path.Moves = right_moves;
                right_path.Overlap = RightOverlap;
                memcpy(right_moves, _path.Moves, _path.Length);
                memcpy(right_path.Overlap, _path.Overlap, sizeof(RightOverlap));

                right_score = Follow_Edge(current_cell,
                    adj_cell,
                    &right_path,
                    FACING_EDGE_RIGHT,
                    direction,
                    threat,
                    threat_state,
                    sizeof(right_moves) - 2,
                    move);

                if (left_score != 0 || right_score != 0) {
                    break;
                }

                while (true) {
                    if (adj_cell == dest) {
                        break;
                    }

                    next_dir = Direction_To_Facing(Cell_Direction8(adj_cell, dest));
                    adj_cell = Cell_Get_Adjacent(adj_cell, next_dir);

                    if (Passable_Cell(adj_cell, FACING_NONE, threat, move) == 0) {
                        ++i;

                        break;
                    }

                    if (adj_cell == dest) {
                        if (threat == -1) {
                            // Set current a break to finish pathfinding.
                            current_cell = adj_cell;

                            break;
                        }

                        switch (threat_state++) {
                            case 0:
                                threat = risk >> 1;
                                break;
                            case 1:
                                threat += risk;
                                break;
                            case 2:
                                threat = -1;
                                break;
                            default:
                                break;
                        }

                        break;
                    }
                }
            }

            if (left_score != 0 || right_score != 0) {
                chosen_path = &left_path;

                if (right_score == 0) {
                    chosen_path = &left_path;
                } else if (left_score != 0) {
                    chosen_path = &right_path;

                } else {
                    chosen_path = left_path.Length >= right_path.Length ? &right_path : &left_path;
                }

                int move_count = std::min(chosen_path->Length, length);

                if (move_count > 0) {
                    memcpy(_path.Overlap, chosen_path->Overlap, sizeof(MainOverlap));
                    memcpy(_path.Moves, chosen_path->Moves, move_count);
                    _path.Length = move_count;
                    _path.Score = chosen_path->Score;
                    _path.PreviousCell = -1;
                    _path.UnravelCheckpoint = -1;

                    current_cell = adj_cell;
                } else {
                    break;
                }
            }
        }
    }

    if (_path.Length < length) {
        _path.Moves[_path.Length++] = FACING_NONE;
    }

    Optimize_Moves(&_path, move);

    return &_path;
}

BOOL FootClass::Basic_Path()
{
    return 0;
}

/**
 * Attempts to remove loops from the path.
 *
 * 0x004BF49C
 */
BOOL FootClass::Unravel_Loop(PathType *path, cell_t &cell, FacingType &facing, int x1, int y1, int x2, int y2, MoveType move)
{
    BOOL even = false;
    cell_t cellnum = cell + AdjacentCell[Opposite_Facing(facing)];
    FacingType *facing_ptr = &path->Moves[path->Length - 1];
    FacingType face = facing;

    for (int i = path->Length; i > 0; --i) {
        if (Point_Relative_To_Line(Cell_Get_X(cellnum), Cell_Get_Y(cellnum), x1, y1, x2, y2) == 0 || even) {
            if ((face % 2) != 0 && cellnum != path->UnravelCheckpoint) {
                cell = cellnum;
                facing = *(facing_ptr - 1); // field_A a pointer to facing types?
                path->UnravelCheckpoint = cellnum;
                path->Length = i;

                return true;
            }

            even = even == false; // flip even bool
        }

        path->Score -= Passable_Cell(cellnum, *facing_ptr, -1, move);
        path->Overlap[cellnum / PATH_FLAG_BITSIZE] &= ~(1 << (cellnum % PATH_FLAG_BITSIZE));
        face = *facing_ptr--;
        cellnum += AdjacentCell[Opposite_Facing(face)];
    }

    return false;
}

/**
 * Registers a cell in the path and check if we visit the cell already.
 *
 * 0x004BF5E0
 */
BOOL FootClass::Register_Cell(PathType *path, cell_t cell, FacingType facing, int cost, MoveType move)
{
    // Check the flagging for the passed in cell, if its not flagged, then add
    // facing to what appears to be the move list.
    if ((path->Overlap[cell / PATH_FLAG_BITSIZE] & (1 << (cell % PATH_FLAG_BITSIZE))) == 0) {
        path->Moves[path->Length++] = facing;
        path->Score += cost;
        path->Overlap[cell / PATH_FLAG_BITSIZE] |= 1 << (cell % PATH_FLAG_BITSIZE);

        return true;
    }

    FacingType last_move = path->Moves[path->Length - 1];

    // If the last facing in the list matches our facing, then do some unflagging
    // vodoo
    if (Opposite_Facing(facing) == last_move) {
        cell_t cellnum = Cell_Get_Adjacent(cell, last_move); // AdjacentCell[last_move] + cell;
        path->Overlap[cellnum / PATH_FLAG_BITSIZE] &= ~(1 << (cellnum % PATH_FLAG_BITSIZE));
        --path->Length;

        return true;
    }

    // If cell doesn't equal the previous cell perhaps?
    if (cell != path->PreviousCell) {
        int count = 0;
        cell_t test_cell = path->StartCell;
        FacingType *face_ptr = path->Moves;
        path->PreviousCell = cell;

        // If the cell isn't the start cell, go through the list and see if any
        // of the directions we currently have lead to the cell.
        if (cell != path->StartCell) {
            for (; count < path->Length; ++count) {
                test_cell += AdjacentCell[*face_ptr++];

                if (test_cell == cell) {
                    break;
                }
            }
        }

        // For any remaining count do more stuff.
        for (int i = count; i < path->Length; ++i) {
            cell_t adj_cell = AdjacentCell[*face_ptr] + test_cell;
            path->Score -= Passable_Cell(adj_cell, *face_ptr, -1, move);
            path->Overlap[adj_cell / PATH_FLAG_BITSIZE] &= ~(1 << (adj_cell % PATH_FLAG_BITSIZE));
            test_cell = adj_cell;
            ++face_ptr;
        }

        path->Length = count;

        return true;
    }

    return false;
}

/**
 * Calculates a path around an obstacle after "crashing" into it during path calc.
 *
 * 0x004BFDE4
 */
BOOL FootClass::Follow_Edge(cell_t start, cell_t destination, PathType *path, FacingType chirality, FacingType facing,
    int threat, int threat_state, int length, MoveType move)
{
    // OpenDUNE calls this function Script_Unit_Pathfinder_Connect. Again the
    // C&C version is more complicated, checking the relative position of
    // the cell being checked against a straight line to the target.

    // If we don't have a path stuct to record the found path to, don't bother.
    if (path == nullptr) {
        return false;
    }

    int start_x = Cell_Get_X(start);
    int start_y = Cell_Get_Y(start);
    int dest_x = Cell_Get_X(destination);
    int dest_y = Cell_Get_Y(destination);
    int last_relative = 0;
    int path_count = 0;
    bool no_last_relative = true;
    bool loop_finished = false;

    path->PreviousCell = -1;
    path->UnravelCheckpoint = -1;

    FacingType edge_face = Facing_Adjust(facing, chirality);
    FacingType last_face = FACING_NONE;

    cell_t edge_cell = Cell_Get_Adjacent(start, edge_face);
    cell_t curr_cell = start;
    cell_t last_cell = -1;

    // Keep adding cells to the path so long as we have space in our buffer.
    while (path->Length < length) {
        int cell_cost;
        edge_face = facing;

        do {
            bool either_side_of_line = false;
            edge_face = Facing_Adjust(edge_face, chirality);

            // Handle Cardinal and Ordinal facings differently as ordinals are
            // a diagonal move.
            if ((edge_face & FACING_ORDINAL_TEST) != 0) {
                cell_t next_adj = Cell_Get_Adjacent(curr_cell, Facing_Adjust(edge_face, chirality));

                if (next_adj == destination) {
                    cell_cost = Passable_Cell(next_adj, Facing_Adjust(edge_face, chirality), threat, move);

                    if (cell_cost != 0) {
                        edge_face = Facing_Adjust(edge_face, chirality);
                        edge_cell = Cell_Get_Adjacent(curr_cell, edge_face);

                        goto break_loop;
                    }
                }

                cell_t chk_cell = Cell_Get_Adjacent(curr_cell, edge_face);

                int relative =
                    Point_Relative_To_Line(Cell_Get_X(chk_cell), Cell_Get_Y(chk_cell), start_x, start_y, dest_x, dest_y);

                // In theory, Point_Relative_To_Line returns -ve for below and
                // +ve for above the line, so the bool shows if the last two
                // checked points are on opposite sides of the line.
                if (relative != 0 && !no_last_relative) {
                    either_side_of_line = (last_relative ^ relative) < 0;
                } else {
                    either_side_of_line = false;
                }

                if (either_side_of_line && Opposite_Facing(edge_face) == path->Moves[path->Length - 1]) {
                    either_side_of_line = false;
                }
            }

            // We are facing the way we started, so we didn't find a route.
            if (edge_face == facing) {
                return false;
            }

            edge_cell = Cell_Get_Adjacent(curr_cell, edge_face);

            if (!either_side_of_line) {
                cell_cost = Passable_Cell(edge_cell, edge_face, threat, move);

                if (cell_cost != 0) {
                    goto break_loop;
                }
            }

        } while (edge_cell != destination);
        loop_finished = true;

    break_loop:
        if (!loop_finished) {
            if (!Register_Cell(path, edge_cell, edge_face, cell_cost, move)) {
                if (!Unravel_Loop(path, edge_cell, edge_face, start_x, start_y, dest_x, dest_y, move)) {
                    return false;
                }

                edge_face = Facing_Adjust(edge_face, chirality * 2);
            }

            int tmp_rel =
                Point_Relative_To_Line(Cell_Get_X(edge_cell), Cell_Get_Y(edge_cell), start_x, start_y, dest_x, dest_y);

            if (tmp_rel != 0) {
                last_relative = tmp_rel;
                no_last_relative = false;
            } else {
                no_last_relative = true;
            }

            // If we can't get round the obstruction in 100 moves, give up?
            if (++path_count == 100) {
                return false;
            }
        }

        // Success! We found a path around the obstacle.
        if (edge_cell == destination) {
            path->Moves[path->Length] = FACING_NONE;
            return true;
        }

        // No progress, we failed.
        if (edge_cell == last_cell && edge_face == last_face) {
            return false;
        }

        if (last_cell == -1) {
            last_cell = edge_cell;
            last_face = edge_face;
        }

        facing = Facing_Adjust(edge_face, chirality * -3);
        curr_cell = edge_cell;
    }

    return false;
}

/**
 * Cleans the path of none optimal moves.
 *
 * 0x004C0130
 */
int FootClass::Optimize_Moves(PathType *path, MoveType move)
{
    // This is Script_Unit_Pathfinder_Smoothen in OpenDUNE, looks like it does
    // exactly the same thing but cell pass check doesn't use move types.
    DEBUG_ASSERT(m_IsActive);
    DEBUG_ASSERT(move != MOVE_NONE);
    // DEBUG_ASSERT_PRINT(move < MOVE_COUNT, "move value is %d which exceed expected %d.\n", move, MOVE_COUNT);

    static FacingType _trans[] = { FACING_NORTH,
        FACING_NORTH,
        FACING_NORTH_EAST,
        FACING_EAST,
        FACING_SOUTH_EAST,
        FACING_FIXUP_MARK,
        FACING_NONE,
        FACING_NORTH };

    if (path == nullptr || path->Moves == nullptr || path->Length == 0) {
        return 0;
    }

    path->Moves[path->Length] = FACING_NONE;
    cell_t cell = path->StartCell;

    // If we have more than 1 move in our move queue, attempt to optimise
    if (path->Length > 1) {
        FacingType *next_move = &path->Moves[1];

        while (*next_move != FACING_NONE) {
            FacingType *last_move = next_move - 1;

            // Adjust last_move back to the last valid move.
            while (*last_move == FACING_FIXUP_MARK && last_move != path->Moves) {
                --last_move;
            }

            if (*last_move == FACING_FIXUP_MARK) {
                ++next_move;
            } else {
                FacingType facing_diff = Reverse_Adjust(*next_move, *last_move);

                // Don't think this bit is possible given the math, original was
                // probably % 8 rather than & 7 for the adjusts.
                if (facing_diff < 0) {
                    facing_diff = Facing_Adjust(facing_diff, FACING_COUNT);
                }

                FacingType trans_move = _trans[facing_diff];

                if (trans_move == FACING_SOUTH_EAST) { // == 3
                    *last_move = FACING_FIXUP_MARK;
                    *next_move++ = FACING_FIXUP_MARK;
                } else {
                    FacingType tmp;

                    if (trans_move != FACING_NORTH) { // != 0
                        if ((*last_move & FACING_NORTH_EAST) != 0) { // == -1 or 1
                            tmp = Facing_Adjust(*last_move, trans_move >= FACING_NORTH ? FACING_NORTH_EAST : FACING_NONE);

                            // This should always be true given the possible
                            // values.
                            if (std::abs((int)trans_move) == 1) {
                                int score = Passable_Cell(Cell_Get_Adjacent(cell, tmp), tmp, -1, move);

                                if (score != 0) {
                                    *next_move = tmp;
                                    *last_move = tmp;
                                }

                                cell = Cell_Get_Adjacent(cell, *last_move);
                                ++next_move;
                                continue;
                            }
                        } else { // == 2
                            tmp = Facing_Adjust(*last_move, trans_move);
                        }

                        *next_move = tmp;
                        *last_move = FACING_FIXUP_MARK;

                        // Adjust last_move back to the last valid move.
                        while (*last_move == FACING_FIXUP_MARK && last_move != path->Moves) {
                            --last_move;
                        }

                        if (*last_move == FACING_FIXUP_MARK) {
                            cell = path->StartCell;
                        } else {
                            cell = Cell_Get_Adjacent(cell, Facing_Adjust(*last_move, FACING_SOUTH));
                        }
                    } else { // == 0
                        cell = Cell_Get_Adjacent(cell, *last_move);
                        ++next_move;
                    }
                }
            }
        }
    }

    path->Score = 0;
    path->Length = 0;
    FacingType *moves = path->Moves;
    cell = path->StartCell;

    // Build the optimised path.
    for (FacingType *m = path->Moves; *m != FACING_NONE; ++m) {
        if (*m == FACING_FIXUP_MARK) {
            continue;
        }

        cell = Cell_Get_Adjacent(cell, *m);
        path->Score += Passable_Cell(cell, *m, -1, move);
        ++path->Length;
        *moves++ = *m;
    }

    ++path->Length;
    *moves = FACING_NONE;

    return path->Length;
}

/**
 * Looks like it finds the safest cell within a range?
 *
 * 0x004C0570
 */
cell_t FootClass::Safety_Point(cell_t start_cell, cell_t end_cell, int start, int end)
{
    FacingType adj_facing = SS_41B710(Opposite_Facing(Direction_To_Facing(Cell_Direction8(start_cell, end_cell))), 1);

    for (int i = start; i < end; ++i) {
        cell_t cell = end_cell;

        for (int j = 0; j < i; ++j) {
            cell = Cell_Get_Adjacent(cell, adj_facing);
        }

        // If adj_facing is ordinal
        if ((adj_facing & 1) != 0) {
            for (int j = 0; j <= 2 * i; ++j) {
                cell = Cell_Get_Adjacent(cell, SS_41B73C(adj_facing, 3));

                if (Can_Enter_Cell(cell)) {
                    return cell;
                }
            }
        } else {
            for (int j = 0; j <= i; ++j) {
                cell = Cell_Get_Adjacent(cell, SS_41B73C(adj_facing, 4));

                if (Can_Enter_Cell(cell)) {
                    return cell;
                }
            }

            for (int j = 0; j <= i; ++j) {
                cell = Cell_Get_Adjacent(cell, SS_41B73C(adj_facing, 2));

                if (Can_Enter_Cell(cell)) {
                    return cell;
                }
            }
        }
    }

    return -1;
}

/**
 * Determine the cost of a move to a cell.
 *
 * 0x004C0570
 */
int FootClass::Passable_Cell(cell_t cell, FacingType facing, int threat, MoveType move)
{
    static int const _value[MOVE_COUNT] = { 1, 1, 3, 8, 10, 0 };

    MoveType canmove = Can_Enter_Cell(cell, facing);

    if (canmove < MOVE_MOVING_BLOCK) {
        if (Distance_To(cell) > 256) {
            move = MOVE_MOVING_BLOCK;
        }
    }

    if (canmove <= move) {
        if (Session.Game_To_Play() != GAME_CAMPAIGN || threat == -1
            || Distance(Cell_To_Coord(cell), Cell_To_Coord(DestLocation)) <= 1280
            || Map.Cell_Threat(cell, Owner()) <= threat) {
            return _value[canmove];
        }
    }

    return 0;
}

/**
 * Helper function to determine how far off a line a point lies?
 *
 * 0x004BF470
 */
int FootClass::Point_Relative_To_Line(int px, int py, int sx, int sy, int ex, int ey)
{
    return (px - ex) * (sy - ey) - (sx - ex) * (py - ey);
}

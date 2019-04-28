/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Base class for most in game objects.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "abstract.h"
#include "building.h"
#include "globals.h"
#include "coord.h"
#include "target.h"
#include <algorithm>

AbstractClass::AbstractClass(RTTIType type, int id) :
    m_IsActive(true),
    m_RTTI(type),
    m_HeapID(id),
    m_Coord(-1),
    m_Height(0)
{
}

AbstractClass::AbstractClass(AbstractClass const &that) :
    m_IsActive(that.m_IsActive),
    m_RTTI(that.m_RTTI),
    m_HeapID(that.m_HeapID),
    m_Coord(that.m_Coord),
    m_Height(that.m_Height)
{
}

cell_t AbstractClass::Get_Cell() const
{
    return Coord_To_Cell(m_Coord);
}

int AbstractClass::Distance_To_Target(target_t target)
{
    int distance = Distance(Center_Coord(), As_Coord(target));
    BuildingClass *bptr = As_Building(target);

    if (bptr != nullptr) {
        distance -= (bptr->Class_Of().Width() + bptr->Class_Of().Height()) * 64;
        distance = std::max(0, distance);
    }

    return distance;
}

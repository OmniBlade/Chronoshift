/**
 * @file
 *
 * @author OmniBlade
 * @author CCHyper
 *
 * @brief Class containing information about warhead behaviour.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "warheadtype.h"
#include "gameini.h"

#ifndef CHRONOSHIFT_STANDALONE
TFixedIHeapClass<WarheadTypeClass> &g_WarheadTypes = Make_Global<TFixedIHeapClass<WarheadTypeClass> >(0x00691600);
#else
TFixedIHeapClass<WarheadTypeClass> g_WarheadTypes;
#endif

const WarheadTypeClass WarheadSA("SA");
const WarheadTypeClass WarheadHE("HE");
const WarheadTypeClass WarheadAP("AP");
const WarheadTypeClass WarheadFire("Fire");
const WarheadTypeClass WarheadHollowPoint("HollowPoint");
const WarheadTypeClass WarheadSuper("Super");
const WarheadTypeClass WarheadOrganic("Organic");
const WarheadTypeClass WarheadNuke("Nuke");
const WarheadTypeClass WarheadMechanical("Mechanical");

WarheadTypeClass::WarheadTypeClass(const char *name) :
    HeapID(g_WarheadTypes.ID(this)),
    Name(name),
    Spread(1),
    Wall(false),
    Wood(false),
    Ore(false),
    UnkBool(false),
    Explosion(0),
    Death(0)
{
    for (ArmorType armor = ARMOR_FIRST; armor < ARMOR_COUNT; ++armor) {
        Verses[armor] = fixed::_1_1;
    }
}

WarheadTypeClass::WarheadTypeClass(WarheadTypeClass const &that) :
    HeapID(that.HeapID),
    Name(that.Name),
    Spread(that.Spread),
    Wall(that.Wall),
    Wood(that.Wood),
    Ore(that.Ore),
    UnkBool(that.UnkBool),
    Explosion(that.Explosion),
    Death(that.Death)
{
    memcpy(Verses, that.Verses, sizeof(Verses));
}

/**
 * 0x0058FA64
 */
void *WarheadTypeClass::operator new(size_t size)
{
    return g_WarheadTypes.Alloc();
}

/**
 * 0x0058FA78
 */
void WarheadTypeClass::operator delete(void *ptr)
{
    g_WarheadTypes.Free(ptr);
}

/**
 * @brief Initialises object heap. Original did this in RulesClass::Heap_Maximums.
 */
void WarheadTypeClass::Init_Heap()
{
    // in the binary, all warhead types are initliased in RulesClass::Heap_Maximums("") and
    // do not have global class instance, so i have moved them here for continuity.
    new WarheadTypeClass(WarheadSA);
    new WarheadTypeClass(WarheadHE);
    new WarheadTypeClass(WarheadAP);
    new WarheadTypeClass(WarheadFire);
    new WarheadTypeClass(WarheadHollowPoint);
    new WarheadTypeClass(WarheadSuper);
    new WarheadTypeClass(WarheadOrganic);
    new WarheadTypeClass(WarheadNuke);
    new WarheadTypeClass(WarheadMechanical);
}

/**
 * @brief Get type enum value from string.
 */
WarheadType WarheadTypeClass::From_Name(const char *name)
{
    DEBUG_ASSERT(name != nullptr);

    if (strcasecmp(name, "<none>") == 0 || strcasecmp(name, "none") == 0) {
        return WARHEAD_NONE;
    }

    if (name != nullptr) {
        for (WarheadType warhead = WARHEAD_FIRST; warhead < WARHEAD_COUNT; ++warhead) {
            if (strcasecmp(name, Name_From(warhead)) == 0) {
                return warhead;
            }
        }
    }

    return WARHEAD_NONE;
}

/**
 * @brief Get string name from type enum value.
 */
const char *WarheadTypeClass::Name_From(WarheadType warhead)
{
    return warhead != WARHEAD_NONE && warhead < WARHEAD_COUNT ? As_Reference(warhead).Get_Name() : "<none>";
}

/**
 * @brief Get reference to object from type enum value.
 */
WarheadTypeClass &WarheadTypeClass::As_Reference(WarheadType warhead)
{
    DEBUG_ASSERT(&g_WarheadTypes[warhead] != nullptr);

    return g_WarheadTypes[warhead];
}

/**
 * @brief Get pointer to object from enum value.
 *
 * 0x0058FA90
 */
WarheadTypeClass *WarheadTypeClass::As_Pointer(WarheadType warhead)
{
    DEBUG_ASSERT(&g_WarheadTypes[warhead] != nullptr);

    if (warhead != WARHEAD_NONE && warhead < WARHEAD_COUNT) {
        return &g_WarheadTypes[warhead];
    }

    return nullptr;
}

/**
 * @brief Read object data from an ini file.
 *
 * 0x0058FAB0
 */
BOOL WarheadTypeClass::Read_INI(GameINIClass &ini)
{
    char verses_buffer[128];
    char verses_format_buffer[128];

    if (ini.Find_Section(Get_Name()) != nullptr) {
        Spread = ini.Get_Int(Get_Name(), "Spread", Spread);
        Wall = ini.Get_Bool(Get_Name(), "Wall", Wall);
        Wood = ini.Get_Bool(Get_Name(), "Wood", Wood);
        Ore = ini.Get_Bool(Get_Name(), "Ore", Ore);
        Explosion = ini.Get_Int(Get_Name(), "Explosion", Explosion);
        Death = ini.Get_Int(Get_Name(), "InfDeath", Death);

        // Build the verses format based on the armor count.
        for (ArmorType armor = ARMOR_FIRST; armor < ARMOR_COUNT; ++armor) {
            strncat(verses_format_buffer, "100%%", sizeof(verses_format_buffer));
            if (armor != (ARMOR_COUNT - 1)) {
                strncat(verses_format_buffer, ",", sizeof(verses_format_buffer));
            }
        }

        if (ini.Get_String(Get_Name(), "Verses", verses_format_buffer, verses_buffer, sizeof(verses_buffer)) > 0) {
            char *value = strtok(verses_buffer, ",");
            for (ArmorType armor = ARMOR_FIRST; (armor < ARMOR_COUNT) && (value != nullptr); ++armor) {
                DEBUG_ASSERT(value != nullptr);
                Verses[armor] = fixed(value);
                value = strtok(nullptr, ",");
            }
        }

        UnkBool = Verses[ARMOR_HEAVY] == fixed::_0_1;

        return true;
    }

    return false;
}

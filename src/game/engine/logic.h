/**
 * @file
 *
 * @author CCHyper
 *
 * @brief Class handling all objects that need per tick logic applying.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef LOGIC_H
#define LOGIC_H

#include "always.h"
#include "layer.h"

class ObjectClass;
class TriggerClass;

class LogicClass : public LayerClass
{
public:
    LogicClass();
    virtual ~LogicClass();

    void AI();

    void Init();

    void Remove(ObjectClass *const object);
    BOOL Submit(ObjectClass *object, BOOL sort = false);

    void Detach(target_t target, int a2);
};

#ifndef CHRONOSHIFT_STANDALONE
#include "hooker.h"
extern LogicClass &Logic;
extern DynamicVectorClass<TriggerClass *> &LogicTriggers;
#else
extern LogicClass Logic;
extern DynamicVectorClass<TriggerClass *> LogicTriggers;
#endif

#endif // LOGIC_H

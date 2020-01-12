/*
 * Copyright (c) 2019 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "gicv2.h"
#include "spinlock.h"
#include "exceptions.h"
#include "utils.h"
#include "platform/interrupt_config.h"

#define IRQ_PRIORITY_HOST       0
#define IRQ_PRIORITY_GUEST      1

#define IRQ_CFGR_SHIFT(id)      (2*((id) % 16))

typedef struct IrqManager {
    RecursiveSpinlock lock;
    ArmGicV2 gic;
    u16 numSharedInterrupts;
    u8 priorityShift;
    u8 numPriorityLevels;
    u8 numCpuInterfaces;
    u8 numListRegisters;
    // Note: we don't store interrupt handlers since we will handle some SGI + uart interrupt(s)...
} IrqManager;

typedef enum ThermosphereSgi {
    ThermosphereSgi_ExecuteFunction = 0,
    ThermosphereSgi_VgicUpdate = 1,

    ThermosphereSgi_Max = 2,
} ThermosphereSgi;

extern IrqManager g_irqManager;

void initIrq(void);
void configureInterrupt(u16 id, u8 prio, bool isLevelSensitive);
bool irqIsGuest(u16 id);
void irqSetAffinity(u16 id, u8 affinityMask);
void handleIrqException(ExceptionStackFrame *frame, bool isLowerEl, bool isA32);

static inline void generateSgiForAllOthers(ThermosphereSgi id)
{
    g_irqManager.gic.gicd->sgir = (1 << 24) | ((u32)id & 0xF);
}

static inline void generateSgiForSelf(ThermosphereSgi id)
{
    g_irqManager.gic.gicd->sgir = (2 << 24) | ((u32)id & 0xF);
}

static inline void generateSgiForList(ThermosphereSgi id, u32 list)
{
    g_irqManager.gic.gicd->sgir = (0 << 24) | (list << 16) | ((u32)id & 0xF);
}

static inline void generateSgiForAll(ThermosphereSgi id)
{
    generateSgiForList(id, MASK(g_irqManager.numCpuInterfaces));
}

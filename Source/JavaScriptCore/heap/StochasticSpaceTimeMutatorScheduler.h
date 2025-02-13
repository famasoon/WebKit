/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#pragma once

#include "MutatorScheduler.h"
#include <wtf/Seconds.h>
#include <wtf/TZoneMalloc.h>
#include <wtf/WeakRandom.h>

namespace JSC {

class Heap;

// The JSC concurrent GC sometimes stops the world in order to stay ahead of it. These deliberate,
// synthetic pauses ensure that the GC won't have to do one huge pause in order to catch up to the
// retreating wavefront. The scheduler is called "space-time" because it links the amount of time
// that the world is paused for to the amount of space that the world allocated since the GC cycle
// began.

class StochasticSpaceTimeMutatorScheduler final : public MutatorScheduler {
    WTF_MAKE_TZONE_ALLOCATED(StochasticSpaceTimeMutatorScheduler);
public:
    StochasticSpaceTimeMutatorScheduler(Heap&);
    ~StochasticSpaceTimeMutatorScheduler() final;
    
    State state() const final;
    
    void beginCollection() final;
    
    void didStop() final;
    void willResume() final;
    void didReachTermination() final;
    void didExecuteConstraints() final;
    void synchronousDrainingDidStall() final;
    
    MonotonicTime timeToStop() final;
    MonotonicTime timeToResume() final;
    
    void log() final;
    
    void endCollection() final;
    
private:
    class Snapshot;
    friend class Snapshot;
    
    double bytesAllocatedThisCycleImpl();
    
    double bytesSinceBeginningOfCycle(const Snapshot&);
    double maxHeadroom();
    double headroomFullness(const Snapshot&);
    double mutatorUtilization(const Snapshot&);
    
    Heap& m_heap;
    State m_state { Normal };
    
    WeakRandom m_random;
    
    Seconds m_minimumPause;
    double m_pauseScale;
    Seconds m_targetPause;
    
    double m_bytesAllocatedThisCycleAtTheBeginning { 0 };
    double m_bytesAllocatedThisCycleAtTheEnd { 0 };
    
    MonotonicTime m_beforeConstraints;
    MonotonicTime m_plannedResumeTime;
};

} // namespace JSC


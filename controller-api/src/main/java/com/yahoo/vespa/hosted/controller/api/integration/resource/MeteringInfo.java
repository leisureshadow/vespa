// Copyright 2019 Oath Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.controller.api.integration.resource;

import com.yahoo.config.provision.ApplicationId;

import java.util.List;
import java.util.Map;

/**
 * @author olaa
 */
public class MeteringInfo {

    private final ResourceAllocation thisMonth;
    private final ResourceAllocation lastMonth;
    private final ResourceAllocation currentSnapshot;
    Map<ApplicationId, List<ResourceSnapshot>> snapshotHistory;

    public MeteringInfo(ResourceAllocation thisMonth, ResourceAllocation lastMonth, ResourceAllocation currentSnapshot, Map<ApplicationId, List<ResourceSnapshot>> snapshotHistory) {
        this.thisMonth = thisMonth;
        this.lastMonth = lastMonth;
        this.currentSnapshot = currentSnapshot;
        this.snapshotHistory = snapshotHistory;
    }

    public ResourceAllocation getThisMonth() {
        return thisMonth;
    }

    public ResourceAllocation getLastMonth() {
        return lastMonth;
    }

    public ResourceAllocation getCurrentSnapshot() {
        return currentSnapshot;
    }

    public Map<ApplicationId, List<ResourceSnapshot>> getSnapshotHistory() {
        return snapshotHistory;
    }

}

// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.hosted.node.admin.integrationTests;

import com.yahoo.config.provision.DockerImage;
import com.yahoo.config.provision.NodeType;
import com.yahoo.vespa.hosted.dockerapi.ContainerName;
import com.yahoo.vespa.hosted.node.admin.configserver.noderepository.NodeAttributes;
import com.yahoo.vespa.hosted.node.admin.configserver.noderepository.NodeSpec;
import com.yahoo.vespa.hosted.node.admin.configserver.noderepository.NodeState;
import org.junit.Test;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.argThat;
import static org.mockito.ArgumentMatchers.eq;

/**
 * @author freva
 */
public class MultiDockerTest {

    @Test
    public void test() {
        try (DockerTester tester = new DockerTester()) {
            addAndWaitForNode(tester, "host1.test.yahoo.com", DockerImage.fromString("image1"));
            NodeSpec nodeSpec2 = addAndWaitForNode(
                    tester, "host2.test.yahoo.com", DockerImage.fromString("image2"));

            tester.addChildNodeRepositoryNode(
                    new NodeSpec.Builder(nodeSpec2)
                            .state(NodeState.dirty)
                            .vcpus(1)
                            .memoryGb(1)
                            .diskGb(1)
                            .build());

            tester.inOrder(tester.docker).deleteContainer(eq(new ContainerName("host2")));
            tester.inOrder(tester.storageMaintainer).archiveNodeStorage(
                    argThat(context -> context.containerName().equals(new ContainerName("host2"))));
            tester.inOrder(tester.nodeRepository).setNodeState(eq(nodeSpec2.hostname()), eq(NodeState.ready));

            addAndWaitForNode(tester, "host3.test.yahoo.com", DockerImage.fromString("image1"));
        }
    }

    private NodeSpec addAndWaitForNode(DockerTester tester, String hostName, DockerImage dockerImage) {
        NodeSpec nodeSpec = new NodeSpec.Builder()
                .hostname(hostName)
                .wantedDockerImage(dockerImage)
                .state(NodeState.active)
                .type(NodeType.tenant)
                .flavor("docker")
                .wantedRestartGeneration(1L)
                .currentRestartGeneration(1L)
                .vcpus(2)
                .memoryGb(4)
                .diskGb(1)
                .build();

        tester.addChildNodeRepositoryNode(nodeSpec);

        ContainerName containerName = ContainerName.fromHostname(hostName);
        tester.inOrder(tester.docker).createContainerCommand(eq(dockerImage), eq(containerName));
        tester.inOrder(tester.docker).executeInContainerAsUser(
                eq(containerName), eq("root"), any(), eq(DockerTester.NODE_PROGRAM), eq("resume"));
        tester.inOrder(tester.nodeRepository).updateNodeAttributes(eq(hostName),
                eq(new NodeAttributes().withDockerImage(dockerImage).withVespaVersion(dockerImage.tagAsVersion())));

        return nodeSpec;
    }
}

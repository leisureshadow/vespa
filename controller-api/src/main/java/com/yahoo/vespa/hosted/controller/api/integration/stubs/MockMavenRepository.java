package com.yahoo.vespa.hosted.controller.api.integration.stubs;

import com.yahoo.component.Version;
import com.yahoo.vespa.hosted.controller.api.integration.maven.ArtifactId;
import com.yahoo.vespa.hosted.controller.api.integration.maven.Metadata;
import com.yahoo.vespa.hosted.controller.api.integration.maven.MavenRepository;

import java.util.List;

/**
 * Mock repository for maven artifacts, that returns a static metadata.
 *
 * @author jonmv
 */
public class MockMavenRepository implements MavenRepository {

    public static final ArtifactId id = new ArtifactId("ai.vespa", "search");

    @Override
    public Metadata getMetadata() {
        return new Metadata(id, List.of(Version.fromString("6.1"),
                                        Version.fromString("6.2")));
    }

}

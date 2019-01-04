// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.vespa.config.server.http.flags;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ArrayNode;
import com.fasterxml.jackson.databind.node.ObjectNode;
import com.yahoo.container.jdisc.HttpResponse;
import com.yahoo.jdisc.Response;
import com.yahoo.vespa.config.server.http.HttpConfigResponse;
import com.yahoo.vespa.flags.FlagDefinition;
import com.yahoo.vespa.flags.json.DimensionHelper;

import java.io.IOException;
import java.io.OutputStream;
import java.util.Comparator;
import java.util.List;

/**
 * @author hakonhall
 */
public class DefinedFlags extends HttpResponse {
    private static ObjectMapper mapper = new ObjectMapper();
    private static final Comparator<FlagDefinition> sortByFlagId =
            (left, right) -> left.getUnboundFlag().id().compareTo(right.getUnboundFlag().id());

    private final List<FlagDefinition> flags;

    public DefinedFlags(List<FlagDefinition> flags) {
        super(Response.Status.OK);
        this.flags = flags;
    }

    @Override
    public void render(OutputStream outputStream) throws IOException {
        ObjectNode rootNode = mapper.createObjectNode();
        flags.stream().sorted(sortByFlagId).forEach(flagDefinition -> {
            ObjectNode definitionNode = rootNode.putObject(flagDefinition.getUnboundFlag().id().toString());
            definitionNode.put("description", flagDefinition.getDescription());
            definitionNode.put("modification-effect", flagDefinition.getModificationEffect());
            ArrayNode dimensionsNode = definitionNode.putArray("dimensions");
            flagDefinition.getDimensions().forEach(dimension -> dimensionsNode.add(DimensionHelper.toWire(dimension)));
        });
        mapper.writeValue(outputStream, rootNode);
    }

    @Override
    public String getContentType() {
        return HttpConfigResponse.JSON_CONTENT_TYPE;
    }
}

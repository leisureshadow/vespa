// Copyright 2018 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
package com.yahoo.config.model.api;

import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * Config model view of an imported machine-learned model.
 *
 * @author bratseth
 */
public interface ImportedMlModel {

    String name();
    String source();
    Optional<String> inputTypeSpec(String input);
    Map<String, String> smallConstants();
    Map<String, String> largeConstants();
    Map<String, String> functions();
    List<ImportedMlFunction> outputExpressions();

}
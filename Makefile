# SPDX-FileCopyrightText: 2026 miure-project
# SPDX-License-Identifier: GPL-3.0-or-later

BUILD_DIR ?= $(CURDIR)/build
DIST_DIR  ?= $(CURDIR)/dist

ARTIFACT := libmiure-cli.so
DIST_ARTIFACT := $(DIST_DIR)/$(ARTIFACT)

CMAKE_PROFILE := Release

ifdef BUILD_DEBUG
CMAKE_PROFILE := Debug
endif


.PHONY: all build clean distclean

.DEFAULT_GOAL := all

all: $(DIST_ARTIFACT)


$(BUILD_DIR)/build.ninja: CMakeLists.txt
	mkdir -p $(BUILD_DIR)

	cmake \
		-B $(BUILD_DIR) \
		-S $(CURDIR) \
		-G Ninja \
		-DCMAKE_BUILD_TYPE=$(CMAKE_PROFILE) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON


build: $(BUILD_DIR)/build.ninja
	cmake --build $(BUILD_DIR)


# ----------------------------------------------------------------------
# Install/copy artifact
# ----------------------------------------------------------------------

$(DIST_ARTIFACT): build
	mkdir -p $(DIST_DIR)
	
	cp \
		$(BUILD_DIR)/$(ARTIFACT) \
		$@


# ----------------------------------------------------------------------
# Cleanup
# ----------------------------------------------------------------------

clean:
	rm -rf $(BUILD_DIR)

distclean:
	rm -rf $(BUILD_DIR) $(DIST_ARTIFACT)

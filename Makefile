BUILD_DIR = build
TARGET_LIB = librtk_graphical.so
TARGET_TEST = graphical_test

all: $(BUILD_DIR)/Makefile
	@$(MAKE) -C $(BUILD_DIR) --no-print-directory
	@cp -a $(BUILD_DIR)/$(TARGET_LIB)* . 2>/dev/null || true
	@if [ -f $(BUILD_DIR)/$(TARGET_TEST) ]; then cp $(BUILD_DIR)/$(TARGET_TEST) .; fi
	@echo "Build success."

$(BUILD_DIR)/Makefile:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake ..

remake: fclean all

clean:
	@if [ -d $(BUILD_DIR) ]; then $(MAKE) -C $(BUILD_DIR) clean --no-print-directory; fi

fclean: clean
	@rm -rf $(BUILD_DIR)
	@rm -f $(TARGET_LIB)* $(TARGET_TEST)
	@echo "fclean success."

test: all
	@echo "Test run"
	@if [ -f ./$(TARGET_TEST) ]; then \
		./$(TARGET_TEST); \
	elif [ -f $(BUILD_DIR)/$(TARGET_TEST) ]; then \
		$(BUILD_DIR)/$(TARGET_TEST); \
	else \
		echo "no test: '$(TARGET_TEST)' find."; \
	fi

format-cmake:
	@echo "run cmake format"
	@cmake-format -i CMakeLists.txt cmake/EmbedSpirv.cmake

.PHONY: all remake clean fclean test format-cmake

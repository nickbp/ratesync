DIRS=src
COMMANDS=all debug clean

define runcmd
 for dir in $(DIRS); do (cd $$dir; $(MAKE) $(1)); done
endef

$(COMMANDS):
	$(call runcmd,$@)

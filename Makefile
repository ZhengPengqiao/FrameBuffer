SUBDIRS	:=  ./

.PHONY: all
all:$(SUBDIRS)
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		echo "Complite $$subdir Folder";\
		$(MAKE) -C $$subdir;\
		echo "$$subdir Folder Complite OK";\
	done


.PHONY: clean
clean:
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		echo "Clean $$subdir Folder";\
		$(MAKE) -C $$subdir clean;\
		echo "$$subdir Folder Clean OK";\
	done

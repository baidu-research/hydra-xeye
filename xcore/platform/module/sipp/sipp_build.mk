########################################################################################
# Add rules for SIPP binaries:
$(svuSippImg).mvlib : $(SippSvuObj)
	@mkdir -p $(dir $@)
	$(ECHO) $(LD) $(MVLIBOPT) $(SippSvuObj) -o $@

$(svuSippImg)_sym.o : $(svuSippImg).shvdcomplete
	$(ECHO) $(OBJCOPY) --prefix-symbols=lrt_SS_ --extract-symbol $< $@

$(DirAppObjBase)$(DirAppRoot)/leon_rt/appMemMap_lrt.o : $(svuSippImg).shvdlib
$(svuSippImg)Map.o : $(svuSippImg).shvdlib
	@mkdir -p $(dir $@) 
	$(OBJCOPY) -I binary $(REVERSE_BYTES) --rename-section .data=.ddr.data \
			--redefine-sym  _binary_$(subst /,_,$(subst .,_,$<))_start=mbinImgSipp \
			-O elf32-littlesparc -B sparc $< $@







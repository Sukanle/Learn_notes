NUMBER_WIDTH := 3

define print_aligned_number
	@printf "[%*d]\n" $(NUMBER_WIDTH) $(1)
endef

.PHONY: print_numbers
print_numbers:
	$(call print_aligned_number,2)
	$(call print_aligned_number,20)
	$(call print_aligned_number,100)

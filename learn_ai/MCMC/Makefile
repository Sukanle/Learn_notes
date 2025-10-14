.PHONY:ALL clean

MAKE_COLOR_NORMAL   := \033[0m       # 设置结束颜色
MAKE_COLOR_BULE 	:= \033[38;5;12m # 设置蓝色，表示构建提示
MAKE_COLOR_PINK 	:= \033[38;5;13m # 设置粉色，表示清理提示
MAKE_COLOR_DRABULE  := \033[38;5;33m # 设置深蓝色，表示构建完成
MAKE_COLOR_DPINK 	:= \033[38;5;177m # 设置深粉色，表示清理完成
ALL:
	@printf '$(MAKE_COLOR_BULE)Make from Monte-Carlo$(MAKE_COLOR_NORMAL)\n'
	@make -C $(PWD)/Monte-Carlo 
	@echo
	@printf '$(MAKE_COLOR_BULE)Make from Markov-Chain$(MAKE_COLOR_NORMAL)\n'
	@make -C $(PWD)/Markov-Chain
	@echo
	@printf '$(MAKE_COLOR_DRABULE)All target has done!$(MAKE_COLOR_NORMAL)\n'

clean:
	@printf '$(MAKE_COLOR_PINK)Clean by Markov-Chain$(MAKE_COLOR_NORMAL)\n'
	@make -C $(PWD)/Monte-Carlo clean
	@echo
	@printf '$(MAKE_COLOR_PINK)Clean by Markov-Chain$(MAKE_COLOR_NORMAL)\n'
	@make -C $(PWD)/Markov-Chain clean
	@echo
	@printf '$(MAKE_COLOR_DPINK)All target has clean!$(MAKE_COLOR_NORMAL)\n'

#!/bin/bash

# 检测是否为Zsh环境
ECHO_NORMAL="\033[0m"
ECHO_DGRUEEN="\033[0;32m"
ECHO_GRUEEN="\033[1;32m"
ECHO_YELLOW="\033[1;33m"
ECHO_RED="\033[1;31m"
ECHO_BLUE="\033[1;34m"
ECHO_PATH="\033[4;36m"
PREFIX="${HOME}/.zshrc"
PREFIX="${HOME}/.local/shell"
FILE="${PREFIX}/learn_note_env.sh"

main() {
  if [ -e "${PREFIX}" ]; then
    echo -n -e "${ECHO_BLUE}[Info]${ECHO_NORMAL} ${ECHO_DGRUEEN}Zsh${ECHO_NORMAL} environment detected,using the "
    echo -e "configuration file: ${ECHO_PATH}${PREFIX}${ECHO_NORMAL}"
  else
    PREFIX="${HOME}/.bashrc"
    echo -n -e "${ECHO_YELLOW}[Warning]${ECHO_NORMAL} Using the default bash environment, the config file: "
    echo -e "${ECHO_PATH}${PREFIX}${ECHO_NORMAL}"
  fi

  if [ ! -d "${PREFIX}" ]; then
    mkdir -p ${PREFIX}
    echo -e "${ECHO_YELLOW}[Warning]${ECHO_NORMAL} $(${PREFIX}) directory not exist, try to create."
  fi

  # 检查环境变量是否已存在
  if grep -q "source ${PREFIX}/${FILE}" "${PREFIX}"; then
    # 如果已存在，更新它
    sed -i.bak "s|source .*/learn_note_env.sh \"${PREFIX}\"|" "${PROFILE}"
    echo -e "learn_note_env.sh script already exists, update the import path settings"
  else
    # 如果不存在，追加到文件末尾
    #
    echo "source ${PREFIX}/${FILE}" | cat >> ${PREFIX}
    echo -e "learn_note_env.sh script does not exist, try to create and configure"
  fi

  # 获取当前目录的绝对路径
  CURRENT_DIR=$(pwd)

  set_update_envvar "LEARN" "${CURRENT_DIR}" "${PREFIX}"
  set_update_envvar "LPY" "\$LEARN/learn_python" "${PREFIX}"
  set_update_envvar "LSH" "\$LEARN/learn_shell" "${PREFIX}"
  set_update_envvar "LCPP" "\$LEARN/learn_cpp" "${PREFIX}"
  set_update_envvar "LTBB" "\$LEARN/learn_tbb" "${PREFIX}"
  set_update_envvar "LXML" "\$LEARN/learn_xml" "${PREFIX}"
  set_update_envvar "LASIO" "\$LEARN/learn_asio" "${PREFIX}"
  set_update_envvar "LMAKE" "\$LCPP/learn_make" "${PREFIX}"
  set_update_envvar "LCMAKE" "\$LCPP/learn_cmake" "${PREFIX}"

  # 提示用户需要重新加载配置文件
  echo -e "${ECHO_BLUE}[Info]${ECHO_NORMAL} Run the following command for the changes to take effect:"
  echo -e "--> \033[4;32msource ${PREFIX}${ECHO_NORMAL}"
}

set_update_envvar() {
  local var_name="$1"
  local var_value="$2"

  # 检查环境变量是否已存在
  if grep -q "export $var_name=" "${PREFIX}"; then
    # 如果已存在，更新它
    sed -i.bak "s|export $var_name=.*|export $var_name=\"$var_value\"|" "${PREFIX}"
    echo -n -e "${ECHO_BLUE}[Info]${ECHO_NORMAL} $ECHO_GRUEEN$var_name (user)${ECHO_NORMAL} environment variables "
    echo -e "have been ${ECHO_DGRUEEN}updated${ECHO_NORMAL}: ${ECHO_PATH}$var_value${ECHO_NORMAL}"
  else
    # 如果不存在，追加到文件末尾
    echo "export $var_name=\"$var_value\"" >>"${PREFIX}"
    echo -n -e "${ECHO_BLUE}[Info]${ECHO_NORMAL} ${ECHO_GRUEEN}$var_name (user)${ECHO_NORMAL} environment variables "
    echo -e "have been ${ECHO_DGRUEEN}added${ECHO_NORMAL}: ${ECHO_PATH}$var_value${ECHO_NORMAL}"
  fi
}

main "$@"

# 基底映像檔
FROM ubuntu:22.04

# 避免安裝過程中的互動式提示 (如時區選擇)
ENV DEBIAN_FRONTEND=noninteractive

# 更新套件清單 & 安裝核心建置工具
# build-essential: 包含 GCC, G++, Make
# cmake: 建置系統
# git: 版本控制
# gdb: 除錯工具
# ninja-build: 比 Make 更快的建置工具 (Google 推薦)
# valgrind: 記憶體洩漏檢測 (規格書 5.4 節暗示需要)
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    gdb \
    ninja-build \
    valgrind \
    && rm -rf /var/lib/apt/lists/*

# 設定工作目錄
WORKDIR /app

# 預設指令 (當容器啟動時執行)
CMD ["/bin/bash"]
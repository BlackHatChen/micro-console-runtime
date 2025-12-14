# 使用 Ubuntu 22.04 LTS 作為基底映像檔 (穩定性高，業界常用)
FROM ubuntu:22.04

# 避免安裝過程中的互動式提示卡住建置
ENV DEBIAN_FRONTEND=noninteractive

# --- 1. 安裝基礎開發工具與交叉編譯器 ---
# build-essential: 包含 GCC, Make 等基礎工具
# cmake: 建置系統
# git: 版本控制
# gdb: 除錯工具
# g++-aarch64-linux-gnu: 針對 ARM64 架構 (如 Raspberry Pi, 手機) 的交叉編譯器 [cite: 143]
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    gdb \
    g++-aarch64-linux-gnu \
    && rm -rf /var/lib/apt/lists/* 
# 清理暫存以減少映像檔大小

# --- 2. 設定工作目錄(當登入容器時，預設會直接進入 /app 資料夾) ---
WORKDIR /app

# --- 3. 預設指令 ---
# 當容器啟動時，預設開啟 Bash 終端機
CMD ["/bin/bash"]
#!/usr/bin/env node

/**
 * 安装前环境变量检查
 * 检查 DEST_DIR 等必需环境变量是否已设置
 */

import path from 'path';
import { fileURLToPath } from 'url';
import { dirname } from 'path';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// 获取当前包的根目录
const packageRoot = path.resolve(__dirname, '..');

// 如果是在包自身目录下执行 npm install（开发模式），跳过检查
// 判断依据：INIT_CWD 与包目录相同，说明是在包源码目录执行
const initCwd = process.env.INIT_CWD || process.cwd();
if (path.resolve(initCwd) === path.resolve(packageRoot)) {
    console.log('Fleet: 开发模式，跳过安装检查');
    process.exit(0);
}

const destDir = process.env.DEST_DIR;

if (!destDir) {
  console.error('Fleet: 请指定 DEST_DIR 环境变量');
  console.error('');
  console.error('  用法: DEST_DIR=<目标目录> BUILD_TYPE=debug npm install');
  console.error('');
  console.error('  环境变量:');
  console.error('    DEST_DIR     必需，目标目录');
  console.error('    BUILD_TYPE   可选，构建类型 (debug/release)，默认 release');
  process.exit(1);
}

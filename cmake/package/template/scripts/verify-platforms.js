#!/usr/bin/env node

/**
 * 在发布前验证所有支持平台的二进制文件都存在
 */

import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// 获取当前包的根目录
const packageRoot = path.resolve(__dirname, '..');
const prebuildsDir = path.join(packageRoot, 'prebuilds');
const packageJsonPath = path.join(packageRoot, 'package.json');

// 读取 package.json
const packageJson = JSON.parse(fs.readFileSync(packageJsonPath, 'utf-8'));

// 支持的平台
const supportedOS = packageJson.os || [];
const supportedCPU = packageJson.cpu || [];

console.log('Fleet: 验证平台二进制文件...');
console.log(`支持的操作系统: ${supportedOS.join(', ')}`);
console.log(`支持的架构: ${supportedCPU.join(', ')}`);

// 检查 prebuilds 目录
if (!fs.existsSync(prebuildsDir)) {
  console.error('错误: prebuilds 目录不存在');
  process.exit(1);
}

// 生成所有需要的平台目录
const requiredPlatforms = [];
for (const os of supportedOS) {
  for (const cpu of supportedCPU) {
    requiredPlatforms.push(`${os}-${cpu}`);
  }
}

// 验证每个平台的二进制文件
let allValid = true;
const missingPlatforms = [];
const existingPlatforms = [];

requiredPlatforms.forEach(platform => {
  const platformDir = path.join(prebuildsDir, platform);

  if (!fs.existsSync(platformDir)) {
    console.error(`缺少: ${platform}/`);
    missingPlatforms.push(platform);
    allValid = false;
  } else {
    const files = fs.readdirSync(platformDir);
    if (files.length === 0) {
      console.error(`空目录: ${platform}/`);
      missingPlatforms.push(platform);
      allValid = false;
    } else {
      console.log(`存在: ${platform}/ (${files.length} 个文件)`);
      existingPlatforms.push(platform);
    }
  }
});

if (!allValid) {
  console.error('\n发布失败: 缺少以下平台的二进制文件:');
  missingPlatforms.forEach(p => console.error(`   - ${p}`));
  console.error('\n请确保所有平台的二进制文件都已编译完成。');
  process.exit(1);
}

console.log('\n所有平台验证通过，可以发布！');
console.log(`   包含平台: ${existingPlatforms.join(', ')}`);

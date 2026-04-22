#!/usr/bin/env node

/**
 * 在 npm install 时自动删除不需要的平台二进制文件，减小包体积
 */

import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// 获取当前包的根目录
const packageRoot = path.resolve(__dirname, '..');
const prebuildsDir = path.join(packageRoot, 'prebuilds');

// 当前平台信息 - 与 install-platform.js 的 getPlatformDirName 保持一致
const platformMap = {
    'win32': 'win32',
    'darwin': 'macos',
    'linux': 'linux'
};
const archMap = {
    'x64': 'x64',
    'arm': 'arm64',
    'arm64': 'aarch64',
    'aarch64': 'aarch64',
    'ia32': 'x86'
};
const currentPlatformDir = `${platformMap[process.platform] || process.platform}-${archMap[process.arch] || process.arch}`;

// 开发模式下跳过清理
const initCwd = process.env.INIT_CWD || process.cwd();
if (path.resolve(initCwd) === path.resolve(packageRoot)) {
    console.log('Fleet: 开发模式，跳过清理');
    process.exit(0);
}

console.log(`Fleet: 清理不需要的平台二进制文件...`);
console.log(`当前平台: ${currentPlatformDir}`);

// 检查 prebuilds 目录是否存在
if (!fs.existsSync(prebuildsDir)) {
  console.log('prebuilds 目录不存在，无需清理');
  process.exit(0);
}

// 检查目录结构：prebuilds/{buildType}/{platformDir}/ 或 prebuilds/{platformDir}/
const entries = fs.readdirSync(prebuildsDir, { withFileTypes: true });
const hasBuildTypeSubdir = entries.some(e => e.isDirectory() && ['debug', 'release'].includes(e.name));

let cleanedSize = 0;

if (hasBuildTypeSubdir) {
  // 目录结构：prebuilds/{buildType}/{platformDir}/
  const buildTypes = ['debug', 'release'];
  buildTypes.forEach(buildType => {
    const buildTypePath = path.join(prebuildsDir, buildType);
    if (!fs.existsSync(buildTypePath)) return;

    const platformDirs = fs.readdirSync(buildTypePath);
    platformDirs.forEach(platformDir => {
      const platformPath = path.join(buildTypePath, platformDir);

      if (!fs.statSync(platformPath).isDirectory()) return;

      // 跳过当前平台的目录
      if (platformDir === currentPlatformDir) {
        console.log(`保留: ${buildType}/${platformDir}`);
        return;
      }

      const dirSize = getDirectorySize(platformPath);
      fs.rmSync(platformPath, { recursive: true, force: true });
      console.log(`删除: ${buildType}/${platformDir} (${formatSize(dirSize)})`);
      cleanedSize += dirSize;
    });
  });
} else {
  // 目录结构：prebuilds/{platformDir}/
  const platformDirs = fs.readdirSync(prebuildsDir);
  platformDirs.forEach(platformDir => {
    const platformPath = path.join(prebuildsDir, platformDir);

    if (!fs.statSync(platformPath).isDirectory()) return;

    // 跳过当前平台的目录
    if (platformDir === currentPlatformDir) {
      console.log(`保留: ${platformDir}`);
      return;
    }

    const dirSize = getDirectorySize(platformPath);
    fs.rmSync(platformPath, { recursive: true, force: true });
    console.log(`删除: ${platformDir} (${formatSize(dirSize)})`);
    cleanedSize += dirSize;
  });
}

if (cleanedSize > 0) {
  console.log(`节省空间: ${formatSize(cleanedSize)}`);
} else {
  console.log(`无需清理，已是最小化安装`);
}

/**
 * 计算目录大小
 */
function getDirectorySize(dirPath) {
  let totalSize = 0;

  function calculateSize(filePath) {
    const stats = fs.statSync(filePath);
    if (stats.isDirectory()) {
      const files = fs.readdirSync(filePath);
      files.forEach(file => {
        calculateSize(path.join(filePath, file));
      });
    } else {
      totalSize += stats.size;
    }
  }

  calculateSize(dirPath);
  return totalSize;
}

/**
 * 格式化文件大小
 */
function formatSize(bytes) {
  if (bytes === 0) return '0 B';
  const k = 1024;
  const sizes = ['B', 'KB', 'MB', 'GB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}

#!/usr/bin/env node

/**
 * 在 npm install 时将运行库拷贝到指定目录
 *
 * 用法: DEST_DIR=<目标目录> BUILD_TYPE=debug npm install
 *
 * 环境变量:
 *   DEST_DIR      必需，目标目录路径（绝对或相对路径）
 *   BUILD_TYPE    可选，构建类型 (debug/release)，默认 release
 *   PLATFORM      可选，平台 (win32/linux/darwin)，默认自动检测
 *   ARCH          可选，架构 (x64/arm64/aarch64/ia32)，默认自动检测
 */

import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { dirname } from 'path';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// 获取当前包的根目录
const packageRoot = path.resolve(__dirname, '..');

/**
 * 解析命令行参数和环境变量
 * @param {string[]} args - 命令行参数数组
 * @returns {object} 解析后的参数对象
 */
function parseArgs(args) {
    const result = {
        destdir: null,
        buildType: 'release',
        platform: process.platform || 'win32',
        arch: process.arch || 'x64'
    };

    // 从环境变量读取
    if (process.env.DEST_DIR) {
        result.destdir = process.env.DEST_DIR;
    }
    if (process.env.BUILD_TYPE) {
        result.buildType = process.env.BUILD_TYPE;
    }
    if (process.env.PLATFORM) {
        result.platform = process.env.PLATFORM;
    }
    if (process.env.ARCH) {
        result.arch = process.env.ARCH;
    }

    // 命令行参数覆盖环境变量
    for (const arg of args) {
        if (arg.startsWith('--destdir=')) {
            result.destdir = arg.split('=')[1];
        } else if (arg.startsWith('--build-type=')) {
            result.buildType = arg.split('=')[1];
        } else if (arg.startsWith('--platform=')) {
            result.platform = arg.split('=')[1];
        } else if (arg.startsWith('--arch=')) {
            result.arch = arg.split('=')[1];
        }
    }

    return result;
}

/**
 * 获取平台对应的目录名
 * @param {string} platform - 平台 (win32/linux/darwin)
 * @param {string} arch - 架构 (x64/arm64/aarch64/ia32)
 * @returns {string} 平台目录名
 */
function getPlatformDirName(platform, arch) {
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

    const plat = platformMap[platform] || platform;
    const a = archMap[arch] || arch;
    return `${plat}-${a}`;
}

/**
 * 解析目标路径
 * @param {string} destdir - 目标目录（绝对或相对路径）
 * @param {string} cwd - 当前工作目录
 * @returns {string} 解析后的绝对路径
 */
function resolveDestPath(destdir, cwd) {
    if (!destdir) {
        return null;
    }

    // 判断是否为绝对路径
    if (path.isAbsolute(destdir)) {
        return destdir;
    }

    // 相对路径则拼接工作目录
    return path.resolve(cwd, destdir);
}

/**
 * 拷贝文件
 * @param {string} src - 源文件路径
 * @param {string} dest - 目标文件路径
 */
function copyFile(src, dest) {
    // 确保目标目录存在
    const destDir = path.dirname(dest);
    if (!fs.existsSync(destDir)) {
        fs.mkdirSync(destDir, { recursive: true });
    }

    fs.copyFileSync(src, dest);
    console.log(`  拷贝: ${path.basename(src)}`);
}

/**
 * 确保目录存在
 * @param {string} dir - 目录路径
 */
function ensureDir(dir) {
    if (!fs.existsSync(dir)) {
        fs.mkdirSync(dir, { recursive: true });
    }
}

/**
 * 递归拷贝目录
 * @param {string} src - 源目录
 * @param {string} dest - 目标目录
 * @returns {number} 拷贝的文件数
 */
function copyDirectory(src, dest) {
    if (!fs.existsSync(src)) return 0;
    ensureDir(dest);
    let count = 0;
    const entries = fs.readdirSync(src, { withFileTypes: true });
    for (const entry of entries) {
        const srcPath = path.join(src, entry.name);
        const destPath = path.join(dest, entry.name);
        if (entry.isDirectory()) {
            count += copyDirectory(srcPath, destPath);
        } else if (entry.isFile()) {
            fs.copyFileSync(srcPath, destPath);
            count++;
        }
    }
    return count;
}

/**
 * 拷贝目录内容（flat 模式，不包含源目录本身）
 * @param {string} src - 源目录
 * @param {string} dest - 目标目录
 * @returns {number} 拷贝的文件数
 */
function copyDirectoryContent(src, dest) {
    if (!fs.existsSync(src)) return 0;
    ensureDir(dest);
    let count = 0;
    const entries = fs.readdirSync(src, { withFileTypes: true });
    for (const entry of entries) {
        const srcPath = path.join(src, entry.name);
        const destPath = path.join(dest, entry.name);
        if (entry.isDirectory()) {
            count += copyDirectory(srcPath, destPath);
        } else if (entry.isFile()) {
            fs.copyFileSync(srcPath, destPath);
            count++;
        }
    }
    return count;
}

/**
 * 获取运行时库文件列表（排除静态库和测试程序）
 * @param {string} libDir - 库文件目录
 * @returns {string[]} 运行时库文件列表
 */
function getRuntimeLibs(libDir) {
    if (!fs.existsSync(libDir)) {
        return [];
    }

    const files = fs.readdirSync(libDir, { withFileTypes: true });
    const runtimeLibs = [];

    for (const file of files) {
        if (!file.isFile()) continue;

        const name = file.name.toLowerCase();
        const ext = path.extname(name);

        // Windows: .dll (动态库), .exe (可执行程序)
        // Linux: .so* (动态库), 无扩展名的可执行文件
        // macOS: .dylib (动态库)

        // 排除静态库 (.lib, .a) 和测试程序
        if (name.endsWith('.lib') || name.endsWith('.a')) {
            continue;
        }

        // 排除测试程序
        if (name.includes('_test') || name.includes('test.')) {
            continue;
        }

        // 包含运行时库
        if (ext === '.dll' || ext === '.exe' ||
            ext === '.so' || name.startsWith('.so') ||
            ext === '.dylib') {
            runtimeLibs.push(path.join(libDir, file.name));
            continue;
        }

        // Linux 无扩展名的可执行文件
        if (!ext) {
            try {
                const stats = fs.statSync(path.join(libDir, file.name));
                // 检查是否为可执行文件
                if (stats.mode & parseInt('111', 8)) {
                    // 排除测试程序
                    if (!file.name.includes('_test')) {
                        runtimeLibs.push(path.join(libDir, file.name));
                    }
                }
            } catch (e) {
                // 忽略
            }
        }
    }

    return runtimeLibs;
}

/**
 * 拷贝 target 类型资源（prebuilds 运行时库）
 * @param {string} destPath - 目标根目录
 * @param {string} buildType - 构建类型
 * @param {string} platformDir - 平台目录名
 * @param {object} res - 资源配置项
 * @returns {number} 拷贝的文件数
 */
function copyTargetResource(destPath, buildType, platformDir, res) {
    const sourceDir = path.join(packageRoot, 'prebuilds', buildType, platformDir);
    if (!fs.existsSync(sourceDir)) {
        console.log(`  跳过: prebuilds/${buildType}/${platformDir} 不存在`);
        return 0;
    }

    const runtimeLibs = getRuntimeLibs(sourceDir);
    if (runtimeLibs.length === 0) {
        console.log(`  跳过: 未找到运行时库文件`);
        return 0;
    }

    const destVal = res.dest || '.';
    const targetDir = path.isAbsolute(destVal) ? destVal : path.join(destPath, destVal);

    let count = 0;
    for (const libPath of runtimeLibs) {
        const destFile = path.join(targetDir, path.basename(libPath));
        try {
            copyFile(libPath, destFile);
            count++;
        } catch (e) {
            console.error(`  错误: 拷贝失败 ${path.basename(libPath)}: ${e.message}`);
        }
    }
    return count;
}

/**
 * 拷贝文件类型资源
 * @param {string} destPath - 目标根目录
 * @param {object} res - 资源配置项
 * @returns {number} 拷贝的文件数
 */
function copyFileResource(destPath, res) {
    const srcPath = path.join(packageRoot, res.source);
    if (!fs.existsSync(srcPath)) {
        console.log(`  跳过: 文件源不存在: ${res.source}`);
        return 0;
    }
    if (!fs.statSync(srcPath).isFile()) {
        return -1; // 不是文件，回退到目录处理
    }
    const destVal = res.dest || path.basename(srcPath);
    const destFile = path.isAbsolute(destVal) ? destVal : path.join(destPath, destVal);
    try {
        copyFile(srcPath, destFile);
        return 1;
    } catch (e) {
        console.error(`  错误: 拷贝文件失败 ${res.source}: ${e.message}`);
        return 0;
    }
}

/**
 * 拷贝目录类型资源（asserts、include 等）
 * @param {string} destPath - 目标根目录
 * @param {object} res - 资源配置项
 * @returns {number} 拷贝的文件数
 */
function copyDirResource(destPath, res) {
    const srcDir = path.join(packageRoot, res.source);
    if (!fs.existsSync(srcDir)) {
        console.log(`  跳过: ${res.type} 源不存在: ${res.source}`);
        return 0;
    }

    const destVal = res.dest || '.';
    const targetDir = path.isAbsolute(destVal) ? destVal : path.join(destPath, destVal);

    let count;
    if (res.flat) {
        console.log(`  拷贝资源(内容): ${res.source} -> ${targetDir}`);
        count = copyDirectoryContent(srcDir, targetDir);
    } else {
        const dirName = path.basename(srcDir);
        const finalDest = path.join(targetDir, dirName);
        console.log(`  拷贝资源: ${res.source} -> ${finalDest}`);
        count = copyDirectory(srcDir, finalDest);
    }
    return count;
}

// 主函数
function main() {
    // 获取 npm install 执行时的工作目录
    // INIT_CWD 是 npm 执行命令的初始目录（项目目录）
    // process.cwd() 是包的目录
    const cwd = process.env.INIT_CWD || process.cwd();

    // 开发模式：在包自身目录下执行 npm install 时跳过
    if (path.resolve(cwd) === path.resolve(packageRoot)) {
        console.log('开发模式，跳过安装');
        return;
    }

    // 解析参数
    const args = parseArgs(process.argv.slice(2));

    // 读取 package.json
    const pkgJsonPath = path.join(packageRoot, 'package.json');
    const pkgJson = JSON.parse(fs.readFileSync(pkgJsonPath, 'utf-8'));
    const pkgName = pkgJson.name || 'unknown';
    const resources = pkgJson.resources || [];
    const targets = pkgJson.targets || [];

    // 从 targets 自动生成 target 类型的资源条目
    const targetResources = targets.map(t => {
        const name = typeof t === 'string' ? t : t.name;
        return { type: 'target', name };
    });
    const allResources = [...targetResources, ...resources];

    console.log(`${pkgName}: 安装平台资源...`);
    console.log(`  工作目录: ${cwd}`);

    // 解析目标路径
    const destPath = resolveDestPath(args.destdir, cwd);

    if (!destPath) {
        console.error('  错误: 必须指定 DEST_DIR 环境变量');
        console.error('');
        console.error('  用法: DEST_DIR=<目标目录> BUILD_TYPE=debug npm install');
        console.error('');
        console.error('  环境变量:');
        console.error('    DEST_DIR     必需，目标目录');
        console.error('    BUILD_TYPE   可选，构建类型 (debug/release)，默认 release');
        console.error('    PLATFORM     可选，平台 (win32/linux/darwin)，默认自动检测');
        console.error('    ARCH         可选，架构 (x64/arm64/aarch64/ia32)，默认自动检测');
        process.exit(1);
    }

    console.log(`  目标目录: ${destPath}`);

    // 确定平台和架构
    const platform = args.platform || process.platform;
    const arch = args.arch || process.arch;
    const platformDir = getPlatformDirName(platform, arch);
    const buildType = args.buildType.toLowerCase();

    console.log(`  平台: ${platformDir}`);
    console.log(`  构建类型: ${buildType}`);

    /**
     * 执行资源拷贝到指定目标目录
     * @param {string} targetDir - 目标根目录
     * @param {object[]} resList - 资源列表
     * @returns {number} 拷贝的文件数
     */
    function copyResources(targetDir, resList) {
        let count = 0;
        for (const res of resList) {
            if (res.type === 'target') {
                count += copyTargetResource(targetDir, buildType, platformDir, res);
            } else {
                const srcPath = path.join(packageRoot, res.source);
                if (fs.existsSync(srcPath) && fs.statSync(srcPath).isFile()) {
                    count += copyFileResource(targetDir, res);
                } else {
                    count += copyDirResource(targetDir, res);
                }
            }
        }
        return count;
    }

    const isPlugin = !!(pkgJson.plugin && pkgJson.plugin.id);

    // 如果没有 resources 配置，使用旧逻辑（仅拷贝 prebuilds 运行时库）
    if (allResources.length === 0) {
        console.log('  无 resources 配置，使用默认安装模式');
        const sourceDir = path.join(packageRoot, 'prebuilds', buildType, platformDir);
        if (!fs.existsSync(sourceDir)) {
            console.error(`  错误: 源目录不存在: ${sourceDir}`);
            process.exit(1);
        }
        const runtimeLibs = getRuntimeLibs(sourceDir);
        if (runtimeLibs.length === 0) {
            console.log(`  警告: 未找到运行时库文件`);
            process.exit(0);
        }
        let count = 0;
        for (const libPath of runtimeLibs) {
            const destFile = path.join(destPath, path.basename(libPath));
            try {
                copyFile(libPath, destFile);
                count++;
            } catch (e) {
                console.error(`  错误: 拷贝失败 ${path.basename(libPath)}: ${e.message}`);
            }
        }
        console.log(`  完成: 拷贝了 ${count} 个文件到 ${destPath}`);
    } else if (isPlugin) {
        // 插件包：资源只拷贝到 plugins/$PLUGIN_ID/，不拷贝到 destPath 根目录
        console.log(`  资源配置: ${allResources.length} 条 (插件模式)`);
    } else {
        // 普通包：按 resources 配置拷贝到 destPath
        console.log(`  资源配置: ${allResources.length} 条`);
        const totalCount = copyResources(destPath, allResources);
        console.log(`  完成: 共拷贝 ${totalCount} 个资源到 ${destPath}`);
    }

    // 插件特殊处理：将动态库和 asserts 内容拷贝到 DEST_DIR/plugins/$PLUGIN_ID/
    if (isPlugin) {
        const pluginId = pkgJson.plugin.id;
        const pluginDestDir = path.join(destPath, 'plugins', pluginId);
        ensureDir(pluginDestDir);

        console.log(`  插件安装: ${pluginId}`);

        // 拷贝动态库到插件目录
        const sourceDir = path.join(packageRoot, 'prebuilds', buildType, platformDir);
        if (fs.existsSync(sourceDir)) {
            const allFiles = fs.readdirSync(sourceDir, { withFileTypes: true });
            for (const file of allFiles) {
                if (!file.isFile()) continue;
                const name = file.name.toLowerCase();
                if (name.endsWith('.dll') || name.endsWith('.so') || name.includes('.so.')) {
                    copyFile(path.join(sourceDir, file.name), path.join(pluginDestDir, file.name));
                }
            }
        }

        // 拷贝 asserts 目录内容到插件目录
        const assertsDir = path.join(packageRoot, 'asserts');
        if (fs.existsSync(assertsDir) && fs.statSync(assertsDir).isDirectory()) {
            const count = copyDirectoryContent(assertsDir, pluginDestDir);
            if (count > 0) {
                console.log(`  插件资源: 拷贝 ${count} 个 asserts 文件到 ${pluginDestDir}`);
            }
        }

        console.log(`  插件文件已拷贝到: ${pluginDestDir}`);
    }
}

// 执行主函数
main();

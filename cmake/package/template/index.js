/**
 * @zkhx/fleet
 * Fleet 基础库 npm 包
 * 提供库元数据查询接口
 */

import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { dirname } from 'path';

// 获取 __dirname 的 ES Module 等价物
const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// 获取包根目录
function getPackageRoot() {
    return __dirname;
}

/**
 * 获取库的当前版本
 * @returns {string} 版本号，格式: major.minor.revision
 */
function getVersion() {
    const packageJson = path.join(getPackageRoot(), 'package.json');
    if (!fs.existsSync(packageJson)) return null;

    const content = JSON.parse(fs.readFileSync(packageJson, 'utf-8'));
    return content.version;
}

/**
 * 获取库支持的构建类型
 * @returns {string[]} 支持的构建类型列表，如: ['debug', 'release']
 */
function getSupportedBuildTypes() {
    const prebuildsDir = path.join(getPackageRoot(), 'prebuilds');
    if (!fs.existsSync(prebuildsDir)) return [];

    const entries = fs.readdirSync(prebuildsDir, { withFileTypes: true });
    const buildTypes = [];

    for (const entry of entries) {
        if (entry.isDirectory()) {
            const buildTypePath = path.join(prebuildsDir, entry.name);
            // 检查该构建类型目录下是否有文件
            const hasFiles = hasFilesRecursive(buildTypePath);
            if (hasFiles) {
                buildTypes.push(entry.name);
            }
        }
    }

    return buildTypes.sort();
}

/**
 * 获取库支持的操作系统
 * @param {string} [buildType] 构建类型 (debug/release)，默认检查所有构建类型
 * @returns {string[]} 支持的操作系统列表，如: ['win32', 'linux', 'darwin']
 */
function getSupportedOS(buildType) {
    const prebuildsDir = path.join(getPackageRoot(), 'prebuilds');
    if (!fs.existsSync(prebuildsDir)) return [];

    const osSet = new Set();

    if (buildType) {
        // 检查指定构建类型
        const buildTypeDir = path.join(prebuildsDir, buildType.toLowerCase());
        if (fs.existsSync(buildTypeDir)) {
            collectOSFromDir(buildTypeDir, osSet);
        }
    } else {
        // 检查所有构建类型
        const buildTypes = getSupportedBuildTypes();
        for (const type of buildTypes) {
            const buildTypeDir = path.join(prebuildsDir, type);
            collectOSFromDir(buildTypeDir, osSet);
        }
    }

    return Array.from(osSet).sort();
}

/**
 * 从目录中收集操作系统类型
 * @param {string} dir - 要扫描的目录
 * @param {Set} osSet - 用于收集操作系统类型的 Set
 */
function collectOSFromDir(dir, osSet) {
    if (!fs.existsSync(dir)) return;

    const entries = fs.readdirSync(dir, { withFileTypes: true });
    for (const entry of entries) {
        if (entry.isDirectory()) {
            // 目录名格式: {os}-{arch}，如 win32-x64, linux-x64, macos-x64
            const dirName = entry.name;
            const osPart = dirName.split('-')[0];

            // 映射目录名到 Node.js platform 名称
            const osMap = {
                'win32': 'win32',
                'macos': 'darwin',
                'linux': 'linux'
            };

            const platformName = osMap[osPart];
            if (platformName && hasFilesRecursive(path.join(dir, dirName))) {
                osSet.add(platformName);
            }
        }
    }
}

/**
 * 获取库支持的硬件架构
 * @param {string} [buildType] 构建类型 (debug/release)，默认检查所有构建类型
 * @returns {string[]} 支持的硬件架构列表，如: ['x64', 'arm64', 'ia32']
 */
function getSupportedArch(buildType) {
    const prebuildsDir = path.join(getPackageRoot(), 'prebuilds');
    if (!fs.existsSync(prebuildsDir)) return [];

    const archSet = new Set();

    if (buildType) {
        // 检查指定构建类型
        const buildTypeDir = path.join(prebuildsDir, buildType.toLowerCase());
        if (fs.existsSync(buildTypeDir)) {
            collectArchFromDir(buildTypeDir, archSet);
        }
    } else {
        // 检查所有构建类型
        const buildTypes = getSupportedBuildTypes();
        for (const type of buildTypes) {
            const buildTypeDir = path.join(prebuildsDir, type);
            collectArchFromDir(buildTypeDir, archSet);
        }
    }

    return Array.from(archSet).sort();
}

/**
 * 从目录中收集硬件架构类型
 * @param {string} dir - 要扫描的目录
 * @param {Set} archSet - 用于收集硬件架构类型的 Set
 */
function collectArchFromDir(dir, archSet) {
    if (!fs.existsSync(dir)) return;

    const entries = fs.readdirSync(dir, { withFileTypes: true });
    for (const entry of entries) {
        if (entry.isDirectory()) {
            // 目录名格式: {os}-{arch}，如 win32-x64, linux-aarch64
            const dirName = entry.name;
            const parts = dirName.split('-');
            if (parts.length >= 2) {
                const archPart = parts[1];

                // 映射目录名到 Node.js arch 名称
                const archMap = {
                    'x64': 'x64',
                    'x86': 'ia32',
                    'aarch64': 'arm64',
                    'arm64': 'arm64'
                };

                const archName = archMap[archPart];
                if (archName && hasFilesRecursive(path.join(dir, dirName))) {
                    archSet.add(archName);
                }
            }
        }
    }
}

/**
 * 递归检查目录下是否有文件
 * @param {string} dir - 目录路径
 * @returns {boolean} 是否有文件
 */
function hasFilesRecursive(dir) {
    if (!fs.existsSync(dir)) return false;

    const entries = fs.readdirSync(dir, { withFileTypes: true });
    for (const entry of entries) {
        if (entry.isFile()) {
            return true;
        } else if (entry.isDirectory()) {
            if (hasFilesRecursive(path.join(dir, entry.name))) {
                return true;
            }
        }
    }
    return false;
}

/**
 * 获取库文件路径
 * @param {string} [buildType] 构建类型 (debug/release)，默认 release
 * @returns {string|null} 库文件目录的绝对路径，不存在则返回 null
 */
function getLibPath(buildType = 'release') {
    const platform = process.platform;
    const arch = process.arch;

    const platformName = getPlatformDirName(platform, arch);
    const prebuildDir = path.join(getPackageRoot(), 'prebuilds', buildType.toLowerCase(), platformName);

    if (!fs.existsSync(prebuildDir)) {
        return null;
    }
    return prebuildDir;
}

/**
 * 获取库文件列表
 * @param {string} [buildType] 构建类型 (debug/release)，默认 release
 * @returns {string[]} 库文件的完整路径列表
 */
function getLibFiles(buildType = 'release') {
    const libPath = getLibPath(buildType);
    if (!libPath) return [];

    const files = fs.readdirSync(libPath, { withFileTypes: true });
    const libFiles = [];

    for (const file of files) {
        if (file.isFile()) {
            const ext = file.name.toLowerCase();
            // Windows: .lib, .dll, .exe
            // Linux: .so, .a, 无扩展名的可执行文件
            if (ext.endsWith('.lib') || ext.endsWith('.dll') || ext.endsWith('.exe') ||
                ext.endsWith('.so') || ext.endsWith('.a') || ext.endsWith('.dylib')) {
                libFiles.push(path.join(libPath, file.name));
            }
        }
    }

    return libFiles;
}

/**
 * 获取平台对应的目录名
 * @param {string} platform - 平台 (win32/linux/darwin)
 * @param {string} arch - 架构 (x64/arm64/aarch64/ia32)
 * @returns {string} 平台目录名，如 win32-x64, linux-x64, macos-x64
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
 * 获取头文件目录路径
 * 优先检查 include 目录，不存在则检查 interface 目录
 * @returns {string|null} 头文件目录的绝对路径，不存在则返回 null
 */
function getIncludePath() {
    const includeDir = path.join(getPackageRoot(), 'include');
    if (fs.existsSync(includeDir)) {
        return includeDir;
    }
    const interfaceDir = path.join(getPackageRoot(), 'interface');
    return fs.existsSync(interfaceDir) ? interfaceDir : null;
}

/**
 * 获取资源文件目录路径
 * @returns {string|null} 资源文件目录的绝对路径，不存在则返回 null
 */
function getAssertsPath() {
    const assertsDir = path.join(getPackageRoot(), 'asserts');
    return fs.existsSync(assertsDir) ? assertsDir : null;
}

/**
 * 检查当前平台是否被支持
 * @param {string} [buildType] 构建类型 (debug/release)，默认检查所有构建类型
 * @returns {boolean} 当前平台是否支持
 */
function isCurrentPlatformSupported(buildType) {
    const os = getSupportedOS(buildType);
    const arch = getSupportedArch(buildType);
    const currentArch = process.arch === 'arm' ? 'arm64' : process.arch;

    return os.includes(process.platform) && arch.includes(currentArch);
}

/**
 * 递归获取目录下所有文件
 * @param {string} dir - 目录路径
 * @returns {string[]} 文件路径列表
 */
function getFilesRecursive(dir) {
    const files = [];
    const entries = fs.readdirSync(dir, { withFileTypes: true });

    for (const entry of entries) {
        const fullPath = path.join(dir, entry.name);
        if (entry.isFile()) {
            files.push(fullPath);
        } else if (entry.isDirectory()) {
            files.push(...getFilesRecursive(fullPath));
        }
    }

    return files;
}

/**
 * 获取库目标名称列表
 * 从 package.json 的 targets 字段提取，过滤掉可执行程序类型
 * 如果同名目标同时以动态库/静态库和可执行程序存在，优先保留动态库/静态库
 * @param {string} [buildType] 构建类型 (debug/release)，默认 release
 * @returns {string[]} 库目标名称列表
 */
function getLibNames(buildType = 'release') {
    const packageJsonPath = path.join(getPackageRoot(), 'package.json');
    if (!fs.existsSync(packageJsonPath)) return [];

    const content = JSON.parse(fs.readFileSync(packageJsonPath, 'utf-8'));
    const targets = content.targets || [];
    if (targets.length === 0) return [];

    // 按 name 分组，同名的以动态库/静态库优先
    const nameMap = new Map();
    for (const t of targets) {
        const name = typeof t === 'string' ? t : t.name;
        const kind = typeof t === 'string' ? '' : (t.kind || '');
        const isLib = kind === 'dynamic' || kind === 'static' || kind === '';

        if (!nameMap.has(name)) {
            nameMap.set(name, { name, kind, isLib });
        } else {
            const existing = nameMap.get(name);
            // 如果现有条目不是库类型且新条目是库类型，则替换
            if (!existing.isLib && isLib) {
                nameMap.set(name, { name, kind, isLib });
            }
        }
    }

    // 过滤掉可执行程序类型
    const libNames = [];
    for (const entry of nameMap.values()) {
        if (entry.kind !== 'execute') {
            libNames.push(entry.name);
        }
    }

    return libNames;
}

/**
 * 获取包信息摘要
 * @param {string} [buildType] 构建类型 (debug/release)，默认检查所有构建类型
 * @returns {object} 包信息对象
 */
function getPackageInfo(buildType) {
    const buildTypes = getSupportedBuildTypes();

    return {
        version: getVersion(),
        supportedBuildTypes: buildTypes,
        supportedOS: getSupportedOS(buildType),
        supportedArch: getSupportedArch(buildType),
        includePath: getIncludePath(),
        assertsPath: getAssertsPath(),
        libPath: {
            release: getLibPath('release'),
            debug: getLibPath('debug')
        },
        currentPlatformSupported: isCurrentPlatformSupported(buildType),
        currentPlatform: getPlatformDirName(process.platform, process.arch)
    };
}

// 导出所有函数
export {
    getVersion,
    getSupportedBuildTypes,
    getSupportedOS,
    getSupportedArch,
    getLibPath,
    getLibFiles,
    getLibNames,
    getPlatformDirName,
    getIncludePath,
    getAssertsPath,
    isCurrentPlatformSupported,
    getPackageInfo
};

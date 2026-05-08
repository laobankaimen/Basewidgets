// install.js - 通用打包脚本
// 读取 rule.json 配置，将资源和构建产物拷贝到打包目录
//
// 参数: node install.js <buildType> <projectRoot> <buildDir>
//         <platformDirPrefix> <libDir> <runtimeDir> <archiveDir> [ruleJsonPath]

const fs = require('fs');
const path = require('path');

const args = process.argv.slice(2);

// 固定参数
const buildType = args[0] || 'Release';
const projectRoot = args[1];
const buildDir = args[2];
const platformDirPrefix = args[3];
const libOutputDir = args[4];
const runtimeOutputDir = args[5];
const archiveOutputDir = args[6];
const ruleJsonPath = args[7] || '';

// ========== 工具函数 ==========

function ensureDir(dir) {
    if (!fs.existsSync(dir)) {
        fs.mkdirSync(dir, { recursive: true });
    }
}

function copyDir(src, dest, pkgConfig, resources, ruleTargets, installResources, pluginConfig) {
    if (!fs.existsSync(src)) {
        console.log('  Warning: Source not found:', src);
        return;
    }
    ensureDir(dest);
    const entries = fs.readdirSync(src, { withFileTypes: true });
    for (const entry of entries) {
        const srcPath = path.join(src, entry.name);
        const destPath = path.join(dest, entry.name);
        if (entry.isDirectory()) {
            copyDir(srcPath, destPath, pkgConfig, resources, ruleTargets, installResources, pluginConfig);
        } else if (entry.name === 'package.json' && pkgConfig) {
            // 处理 package.json 模板
            processTemplateFile(srcPath, destPath, pkgConfig, resources, ruleTargets, installResources, pluginConfig);
        } else {
            copyFileIfNeeded(srcPath, destPath);
        }
    }
}

// 拷贝文件（仅在目标不存在或不同时拷贝）
function copyFileIfNeeded(srcPath, destPath) {
    if (fs.existsSync(destPath)) {
        const srcStat = fs.statSync(srcPath);
        const destStat = fs.statSync(destPath);
        // 比较大小和修改时间，相同则跳过
        if (srcStat.size === destStat.size && srcStat.mtimeMs <= destStat.mtimeMs) {
            console.log('  Skipped (unchanged):', path.basename(srcPath));
            return;
        }
    }
    fs.copyFileSync(srcPath, destPath);
    console.log('  Copied:', srcPath, '->', destPath);
}

// 处理模板文件（替换占位符）
function processTemplateFile(srcPath, destPath, pkgConfig, resources, ruleTargets, installResources, pluginConfig) {
    if (!pkgConfig) {
        // 没有 package 配置，直接拷贝
        copyFileIfNeeded(srcPath, destPath);
        return;
    }

    // 解析模板 JSON
    const template = JSON.parse(fs.readFileSync(srcPath, 'utf-8'));

    // 处理 plugin 段占位符
    if (template.plugin) {
        if (pluginConfig && pluginConfig.isPlugin && pluginConfig.pluginId) {
            const segments = pluginConfig.pluginId.split('.');
            const lastSegment = segments[segments.length - 1];
            const pluginName = lastSegment.charAt(0).toUpperCase() + lastSegment.slice(1);
            if (template.plugin.name === '@pluginName@') template.plugin.name = pluginName;
            if (template.plugin.id === '@pluginId@') template.plugin.id = pluginConfig.pluginId;
            if (template.plugin.groupIds === '@groupIds@') template.plugin.groupIds = pluginConfig.groupIds || [];
        } else {
            delete template.plugin;
        }
    }

    // 必要字段检查
    const requiredFields = ['name', 'version'];
    for (const key of requiredFields) {
        const placeholder = `@${key}@`;
        if (template[key] === placeholder && !pkgConfig[key]) {
            throw new Error(`Required field '${key}' is missing in rule.json package config`);
        }
    }

    // 替换必要字段
    for (const key of requiredFields) {
        const placeholder = `@${key}@`;
        if (template[key] === placeholder) {
            template[key] = pkgConfig[key];
        }
    }

    // 可选字段：有值则替换，无值则删除键
    const optionalStringFields = ['description', 'author'];
    for (const key of optionalStringFields) {
        const placeholder = `@${key}@`;
        if (template[key] === placeholder) {
            if (pkgConfig[key]) {
                template[key] = pkgConfig[key];
            } else {
                delete template[key];
            }
        }
    }

    // repository.url
    if (template.repository && template.repository.url === '@repository@') {
        if (pkgConfig.repository) {
            template.repository.url = pkgConfig.repository;
        } else {
            delete template.repository;
        }
    }

    // 动态生成 files 字段
    if (JSON.stringify(template.files) === JSON.stringify('@files@')) {
        template.files = generateFilesList(resources);
    }

    // 注入 targets 字段
    if (JSON.stringify(template.targets) === JSON.stringify('@targets@')) {
        template.targets = (ruleTargets || []).map(t => typeof t === 'string' ? {name: t} : t);
    }

    // 注入 resources 字段（仅放入非 target 类型的 installResources）
    if (JSON.stringify(template.resources) === JSON.stringify('@resources@')) {
        const autoResources = [];

        // 只合并非 target 类型的 installResources
        if (installResources && installResources.length > 0) {
            for (const r of installResources) {
                if (r.type !== 'target') {
                    autoResources.push(r);
                }
            }
        }

        if (autoResources.length > 0) {
            template.resources = autoResources;
        } else {
            delete template.resources;
        }
    }

    // 序列化为 JSON（格式化输出）
    const content = JSON.stringify(template, null, 2) + '\n';

    // 检查内容是否变化
    if (fs.existsSync(destPath)) {
        const existingContent = fs.readFileSync(destPath, 'utf-8');
        if (content === existingContent) {
            console.log('  Skipped (template unchanged):', path.basename(srcPath));
            return;
        }
    }

    // 写入处理后的内容
    ensureDir(path.dirname(destPath));
    fs.writeFileSync(destPath, content, 'utf-8');
    console.log('  Processed template:', srcPath, '->', destPath);
}

// 根据 resources 配置生成 files 列表
function generateFilesList(resources) {
    const files = [];

    for (const res of resources) {
        const dest = res.dest || '.';
        const isFlat = res.flat === true;

        if (isFlat) {
            // flat 模式：拷贝目录内容，需要列出目录下的文件/子目录
            // 这里简化处理，直接添加 dest 目录（如果 dest 是 '.' 则添加各子目录）
            if (dest === '.') {
                // 需要知道实际拷贝了哪些文件，这里用约定方式
                // template 目录下有 index.js, package.json, scripts/
                const srcName = path.basename(res.source);
                if (srcName === 'template') {
                    files.push('index.js');
                    files.push('scripts/');
                }
            } else {
                files.push(dest.replace(/\/$/, '') + '/');
            }
        } else {
            // 非flat：目录整体拷贝或单个文件
            const source = res.source;
            if (source.includes('asserts')) {
                files.push('asserts/');
            } else if (source.includes('include')) {
                files.push('include/');
            } else if (source.includes('README.md')) {
                files.push('README.md');
            } else if (source.includes('prebuilds')) {
                files.push('prebuilds/');
            } else {
                // 其他文件/目录按 dest 路径添加
                const destPath = dest === '.' ? path.basename(source) : dest;
                if (destPath.endsWith('/')) {
                    files.push(destPath);
                } else {
                    files.push(destPath);
                }
            }
        }
    }

    // 总是包含的文件
    if (!files.includes('index.js')) files.push('index.js');
    if (!files.includes('prebuilds/')) files.push('prebuilds/');
    if (!files.includes('scripts/')) files.push('scripts/');

    return files;
}

// ========== 平台检测 ==========

const platformMap = {
    'win32': 'win32-x64',
    'linux': 'linux-x64',
    'linux-aarch64': 'linux-aarch64',
    'darwin': 'macos-x64',
    'darwin-arm64': 'macos-x64'
};
const platformDirName = platformMap[process.platform] || `linux-${process.arch}`;
const isWindows = process.platform === 'win32';

// ========== 读取 rule.json ==========

let rule = { targets: [], resources: [] };

if (ruleJsonPath && fs.existsSync(ruleJsonPath)) {
    rule = JSON.parse(fs.readFileSync(ruleJsonPath, 'utf-8'));
    console.log('Rule file:', ruleJsonPath);
} else {
    // 兼容：从环境变量读取 targets（旧调用方式）
    const envTargets = (process.env.TARGET_NAMES || '').split(';').filter(n => n);
    if (envTargets.length > 0) {
        rule.targets = envTargets;
    }
    console.log('Rule file: not found, using defaults');
}

// 优先使用 publishTargets，回退到 targets
const rawTargets = rule.publishTargets && rule.publishTargets.length > 0 ? rule.publishTargets : (rule.targets || []);
const targetNames = rawTargets.map(t => typeof t === 'string' ? t : t.name);
const resources = rule.publishResources || [];
const installResources = rule.installResources || [];

// 处理 outputDir 配置
let outputDir;
if (rule.outputDir) {
    if (path.isAbsolute(rule.outputDir)) {
        // 绝对路径直接使用
        outputDir = rule.outputDir;
    } else {
        // 相对路径相对于项目根目录
        outputDir = path.resolve(projectRoot, rule.outputDir);
    }
} else {
    // 默认：buildDir/package
    outputDir = path.join(buildDir, 'package');
}

console.log('=== Install Configuration ===');
console.log('Build Type:', buildType);
console.log('Project Root:', projectRoot);
console.log('Build Directory:', buildDir);
console.log('Output Directory:', outputDir);
console.log('Targets:', targetNames);
console.log('Resources:', resources.length, 'entries');
console.log('');

// ========== 执行打包 ==========
const pluginConfig = rule.isPlugin ? {
    isPlugin: true,
    pluginId: rule.pluginId,
    groupIds: rule.groupIds || []
} : null;

const prebuildDestDir = path.join(outputDir, 'prebuilds', buildType.toLowerCase(), platformDirName);

// 只清空当前 buildType 对应的 prebuilds 子目录（保留其他 buildType 的产物）
if (fs.existsSync(prebuildDestDir)) {
    console.log('--- Cleaning prebuilds for', buildType, '---');
    try {
        fs.rmSync(prebuildDestDir, { recursive: true, force: true });
    } catch (e) {
        console.log('  Warning: Failed to clean prebuilds:', e.message);
    }
}

ensureDir(outputDir);

// 1. 按 rule.json 的 resources 配置拷贝资源
console.log('--- Copying resources ---');
for (const res of resources) {
    const srcPath = path.resolve(projectRoot, res.source);
    const destPath = path.resolve(outputDir, res.dest);

    if (!fs.existsSync(srcPath)) {
        console.log('  Warning: Resource not found:', srcPath);
        continue;
    }

    if (fs.statSync(srcPath).isFile()) {
        // 模式1：文件拷贝（支持重命名/改路径）
        // source: "A"  dest: "dir1/B"  →  $ROOT/A → $DEST/dir1/B
        ensureDir(path.dirname(destPath));
        copyFileIfNeeded(srcPath, destPath);
    } else if (res.flat) {
        // 模式3：目录内容拷贝（flat）
        // source: "asserts"  dest: "dir2"  flat: true  →  $ROOT/asserts/* → $DEST/dir2/
        const isTemplate = srcPath.includes('template');
        copyDir(srcPath, destPath, isTemplate ? rule.package : null, resources, rawTargets, installResources, pluginConfig);
    } else {
        // 模式2：目录整体拷贝
        // source: "asserts"  dest: "."  →  $ROOT/asserts → $DEST/asserts/
        // source: "asserts"  dest: "dir2"  →  $ROOT/asserts → $DEST/dir2/asserts/
        const dirName = path.basename(srcPath);
        const isTemplate = srcPath.includes('template');
        copyDir(srcPath, path.join(destPath, dirName), isTemplate ? rule.package : null, resources, rawTargets, installResources, pluginConfig);
    }
}
console.log('');

// 2. 拷贝构建产物到 prebuilds 目录
console.log('--- Copying build targets to prebuilds ---');
console.log('  Prebuilds destination:', prebuildDestDir);

ensureDir(prebuildDestDir);

// 优先使用 CMake 生成的 target-map.json（包含精确的输出文件路径）
const targetMapPath = path.join(buildDir, `target-map-${buildType}.json`);
let usedTargetMap = false;

if (fs.existsSync(targetMapPath)) {
    try {
        const targetMap = JSON.parse(fs.readFileSync(targetMapPath, 'utf-8'));
        console.log(`  Using target-map.json (${targetMap.length} entries)`);

        // 只拷贝 rule.json 中定义的目标，去重
        const targetNameSet = new Set(targetNames);
        const copiedFiles = new Set();
        for (const entry of targetMap) {
            if (!targetNameSet.has(entry.name)) continue;
            if (!entry.file || entry.file === '') continue;
            const normalizedFile = path.normalize(entry.file);
            if (copiedFiles.has(normalizedFile)) continue;
            if (!fs.existsSync(entry.file)) {
                console.log(`  Warning: File not found: ${entry.file}`);
                continue;
            }
            copiedFiles.add(normalizedFile);
            const destName = path.basename(entry.file);
            copyFileIfNeeded(entry.file, path.join(prebuildDestDir, destName));

            // 根据 CMake 规则查找 companion 文件（Windows .dll → .lib，Linux .so → .a）
            // 规则：.lib/.a 在 ARCHIVE_OUTPUT_DIRECTORY/${CONFIG}/ 下
            //       如果 ARCHIVE_OUTPUT_DIRECTORY 未设置，默认为 target 的 BINARY_DIR
            const baseName = destName.replace(/\.(dll|so.*|dylib|exe)$/i, '');
            const archiveDir = entry.archiveDir || '';
            if (archiveDir) {
                if (isWindows) {
                    const companionName = baseName + '.lib';
                    // 多配置生成器：archiveDir/${CONFIG}/companionName
                    const candidates = [
                        path.join(archiveDir, buildType, companionName),
                        path.join(archiveDir, companionName)
                    ];
                    for (const libPath of candidates) {
                        if (fs.existsSync(libPath)) {
                            const normLib = path.normalize(libPath);
                            if (!copiedFiles.has(normLib)) {
                                copiedFiles.add(normLib);
                                copyFileIfNeeded(libPath, path.join(prebuildDestDir, companionName));
                            }
                            break;
                        }
                    }
                } else {
                    const companionName = 'lib' + baseName + '.a';
                    const candidates = [
                        path.join(archiveDir, companionName),
                        path.join(archiveDir, buildType, companionName)
                    ];
                    for (const aPath of candidates) {
                        if (fs.existsSync(aPath)) {
                            const normA = path.normalize(aPath);
                            if (!copiedFiles.has(normA)) {
                                copiedFiles.add(normA);
                                copyFileIfNeeded(aPath, path.join(prebuildDestDir, companionName));
                            }
                            break;
                        }
                    }
                }
            }
        }
        console.log(`  Copied ${copiedFiles.size} files from target-map`);
        usedTargetMap = true;
    } catch (e) {
        console.log(`  Warning: Failed to parse target-map.json: ${e.message}`);
    }
}

// 回退：通过目录搜索 + 匹配名拷贝
if (!usedTargetMap) {
    console.log('  Fallback to directory scan (target-map.json not found)');

    const targetMatchNames = {};
    for (const t of rawTargets) {
        const name = typeof t === 'string' ? t : t.name;
        const outputName = (typeof t === 'object' && t.outputName) ? t.outputName : null;
        targetMatchNames[name] = outputName ? [name, outputName] : [name];
    }
    console.log('  Target match names:', targetMatchNames);

    const sourceDirs = [];
    if (libOutputDir) sourceDirs.push(libOutputDir);
    if (runtimeOutputDir) sourceDirs.push(runtimeOutputDir);
    if (archiveOutputDir) sourceDirs.push(archiveOutputDir);

    if (platformDirPrefix) {
        sourceDirs.push(path.join(buildDir, 'lib', platformDirPrefix));
        sourceDirs.push(path.join(buildDir, 'bin', platformDirPrefix));
    }

    const configTypes = ['Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel'];
    for (const config of configTypes) {
        sourceDirs.push(path.join(buildDir, 'lib', config));
        sourceDirs.push(path.join(buildDir, 'bin', config));
        sourceDirs.push(path.join(buildDir, 'src', config));
    }
    sourceDirs.push(path.join(buildDir, 'lib'));
    sourceDirs.push(path.join(buildDir, 'bin'));
    sourceDirs.push(path.join(buildDir, 'src'));

    const uniqueSourceDirs = [...new Set(sourceDirs)].filter(dir => dir && fs.existsSync(dir));

    const copyAll = targetNames.length === 0;

    for (const srcDir of uniqueSourceDirs) {
        const files = fs.readdirSync(srcDir);
        for (const file of files) {
            const filePath = path.join(srcDir, file);

            let shouldCopy = copyAll;
            if (!copyAll) {
                for (const [targetName, matchNames] of Object.entries(targetMatchNames)) {
                    for (const matchName of matchNames) {
                        if (file.startsWith(matchName)) {
                            shouldCopy = true;
                            break;
                        }
                    }
                    if (shouldCopy) break;
                }
            }
            if (!shouldCopy) continue;

            if (isWindows) {
                if (file.match(/\.(lib|dll|exe)$/i)) {
                    copyFileIfNeeded(filePath, path.join(prebuildDestDir, file));
                }
            } else {
                if (file.match(/\.(so(\.\d+)*|a|dylib)$/i)) {
                    copyFileIfNeeded(filePath, path.join(prebuildDestDir, file));
                } else {
                    try {
                        const stats = fs.statSync(filePath);
                        if (stats.isFile() && (stats.mode & parseInt('111', 8))) {
                            copyFileIfNeeded(filePath, path.join(prebuildDestDir, file));
                        }
                    } catch (e) { /* ignore */ }
                }
            }
        }
    }
}
console.log('');

// 3. 强制重新处理 template 目录（确保 package.json 等模板文件总是最新的）
const templateDir = path.resolve(projectRoot, 'cmake', 'package', 'template');
if (fs.existsSync(templateDir)) {
    console.log('--- Processing template files ---');
    copyDir(templateDir, outputDir, rule.package || null, resources, rawTargets, installResources, pluginConfig);
    console.log('  Template processed from:', templateDir);
} else {
    console.log('Warning: No template directory found at', templateDir);
}

// 4. 为插件项目自动生成 manifest.json
if (rule.isPlugin && rule.pluginId) {
    console.log('--- Generating manifest.json for plugin ---');

    const pluginId = rule.pluginId;
    const segments = pluginId.split('.');
    const lastSegment = segments[segments.length - 1];
    const pluginName = lastSegment.charAt(0).toUpperCase() + lastSegment.slice(1);

    const pkg = rule.package || {};
    const groupIds = rule.groupIds || [];

    const manifest = {
        PluginName: pluginName,
        PluginId: pluginId,
        PluginVersion: pkg.version || '1.0.0',
        PluginCopyRight: pkg.author || '',
        PluginVendor: pkg.author || '',
        PluginDescription: pkg.description || '',
        PluginAuthor: pkg.author || '',
        IsEssentialPlugin: false,
        PluginUpdateUrl: '',
        GroupInstance: groupIds.map(id => ({ GroupId: id }))
    };

    const manifestDir = path.join(outputDir, 'asserts');
    ensureDir(manifestDir);
    const manifestPath = path.join(manifestDir, 'manifest.json');
    const manifestContent = JSON.stringify(manifest, null, '\t') + '\n';

    fs.writeFileSync(manifestPath, manifestContent, 'utf-8');
    console.log('  Generated:', manifestPath);

    // 将 manifest 注入到生成的 package.json 的 resources 中（供 install-platform.js 使用）
    const generatedPkgPath = path.join(outputDir, 'package.json');
    if (fs.existsSync(generatedPkgPath)) {
        try {
            const generatedPkg = JSON.parse(fs.readFileSync(generatedPkgPath, 'utf-8'));
            if (!generatedPkg.resources) {
                generatedPkg.resources = [];
            }

            const manifestResDest = `${pluginId}/manifest.json`;
            const hasManifestRes = generatedPkg.resources.some(r => r.dest === manifestResDest);
            if (!hasManifestRes) {
                generatedPkg.resources.push({
                    type: 'file',
                    source: 'asserts/manifest.json',
                    dest: manifestResDest
                });
            }

            fs.writeFileSync(generatedPkgPath, JSON.stringify(generatedPkg, null, 2) + '\n', 'utf-8');
            console.log('  Injected manifest resource into package.json');
        } catch (e) {
            console.log('  Warning: Failed to inject manifest resource:', e.message);
        }
    }
}

console.log('=== Installation Complete ===');
console.log('Package location:', outputDir);

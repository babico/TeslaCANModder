const path = require('path');
const { getDefaultConfig } = require('expo/metro-config');

const projectRoot = __dirname;
const workspaceRoot = path.resolve(projectRoot, '..');

const config = getDefaultConfig(projectRoot);

// Allow Metro to bundle .md files as assets
config.resolver.assetExts.push('md');

// Watch the monorepo root so Metro can resolve ../../docs/ imports
config.watchFolders = [workspaceRoot];

// Ensure node_modules resolve from the mobile folder (not the workspace root)
config.resolver.nodeModulesPaths = [path.resolve(projectRoot, 'node_modules')];

module.exports = config;

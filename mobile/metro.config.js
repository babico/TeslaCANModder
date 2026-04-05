const { getDefaultConfig } = require('expo/metro-config');

const config = getDefaultConfig(__dirname);

// Allow Metro to bundle .md files as assets
config.resolver.assetExts.push('md');

module.exports = config;

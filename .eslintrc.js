module.exports = {
  env: {
    browser: false,
    commonjs: true,
    node: true,
  },
  extends: ['airbnb-base', 'plugin:prettier/recommended'],
  ignorePatterns: ['node_modules/'],
  rules: {
    'global-require': 0,
    'no-console': 0,
  },
};

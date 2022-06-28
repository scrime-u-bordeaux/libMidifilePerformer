const path = require('path');

const serverConfig = {
  target: 'node',
  mode: 'development',
  entry: './js/node.index.js',
  output: {
    path: path.resolve(__dirname, 'dist/node'),
    filename: 'midifile-performer.js',
    library: {
      name: 'MidifilePerformer',
      type: 'umd',
    },
    globalObject: 'this',
  },
};

const clientConfig = {
  target: 'web',
  mode: 'production',
  cache: false,
  entry: './js/browser.index.js',
  // entry: {
  //   main: './js/browser.index.js',
  //   wasm: './dist/browser/MidifilePerformer.wasm',
  // },
  output: {
    path: path.resolve(__dirname, 'dist/browser'),
    filename: 'midifile-performer.js',
    // filename: '[name].js',
    library: {
      // name: 'MidifilePerformer',
      // type: 'umd',
      // umdNamedDefine: true,
      // export: 'default',
      type: 'module',
    },
    globalObject: 'this',
  },
  module: {
    rules: [
      {
        test: /\.m?js$/,
        exclude: /(node_modules)/,
        use: {
          loader: 'babel-loader',
          options: {
            presets: [
              '@babel/preset-env',
              // {
              //   plugins: [
              //     '@babel/plugin-transform-runtime',
              //     '@babel/plugin-proposal-class-properties',
              //   ],
              // }
            ],
          },
        },
      },
    ]
  },
  experiments: {
    outputModule: true,
    // asyncWebAssembly: true,
    // syncWebAssembly: true,
  },
};

module.exports = [ serverConfig, clientConfig ];

/*
module.exports = {
  entry: './src/index.js',
  output: {
    path: path.resolve(__dirname, 'dist'),
    filename: 'midifile-performer.js',
    library: {
      name: 'midifilePerformer',
      type: 'umd',
    },
    globalObject: 'this',
  },
  resolve: {
    fallback: {
        // we don't need those in the library
        crypto: false,
        fs: false,
        path: false
    }
  },
  experiments: {
    asyncWebAssembly: true,
  },
};
//*/
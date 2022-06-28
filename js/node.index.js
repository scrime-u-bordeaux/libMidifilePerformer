const mfp = require('../dist/node/MidifilePerformer.js');

class MidifilePerformer {
    constructor() {
        // this._mfp = null;
        this._initialized = false;
    }

    async init() {
        return new Promise((resolve, reject) => {
            // console.log('requiring midifile-performer');
            // this._mfp = require('../js/MidifilePerformer');
            // console.log(this._mfp);
            mfp.onRuntimeInitialized = () => {
                this._initialized = true;
                resolve();
            }
        });
    }

    createRenderer() {
        if (this._initialized) {
            return new mfp.Renderer();
        }
    }
};

module.exports = new MidifilePerformer();

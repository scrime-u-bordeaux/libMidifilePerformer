import MidifilePerformer from '../dist/browser/MidifilePerformer.js';

class MFP {
    constructor() {
        this._mfp = null;
        this._initialized = false;
    }

    async init() {
        return new Promise((resolve, reject) => {
            MidifilePerformer().then((mfp) => {
                this._mfp = mfp;
                this._initialized = true;
                resolve();
            });
        });
    }

    createRenderer() {
        if (this._initialized) {
            return new this._mfp.Renderer();
        }
    }
};

const mfp = new MFP();
// export default mfp;
export { mfp as default };

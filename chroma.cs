using System;

class Chroma {
	public double Re { get; private set; } = 0.0;
	public double Im { get; private set; } = 0.0;
	public double BaseLevel { get; private set; } = 0.3;

	const double SYNC_LIMIT = 0.09;
	const int SYNC_CYCLES = 10;

	int mSyncSamples = 0;
	int mSyncCycles = 0;
	double mInputDelay = 0.0;
	double mDiffDelay = 0.0;
	double mDeltaRe = 0.0;
	double mDeltaIm = 0.0;
	double mOscRe = 0.0;
	double mOscIm = 0.0;
	double mRMS = 0.0;

	public void Sync() {
		mSyncCycles = 0;
		mSyncSamples = 0;
	}

	public void Step() {

	}

	public void Step(double signal) {
		var diff = signal - mInputDelay;
		mInputDelay = signal;
		mRMS = mRMS * 0.9 + diff * 0.1;

		if (SYNC_CYCLES <= mSyncCycles) {
			var delayRe = mOscRe;
			var delayIm = mOscIm;
			mOscRe = delayRe * mDeltaRe - delayIm * mDeltaIm;
			mOscIm = delayRe * mDeltaIm + delayIm * mDeltaRe;
			Re = (mDiffDelay * mOscIm - diff * delayIm) / BaseLevel;
			Im = (mDiffDelay * mOscRe - diff * delayRe) / BaseLevel;
			mDiffDelay = diff;
			return;
		}

		if (SYNC_LIMIT <= mRMS && diff < 0.0 && 0.0 <= mDiffDelay) {
			if (0 == mSyncCycles) {
				mSyncSamples = 0;
			}
			mSyncCycles++;
		}
		mSyncSamples++;
		mDiffDelay = diff;

		if (SYNC_CYCLES <= mSyncCycles) {
			var freq = (mSyncCycles - 1.0) / mSyncSamples;
			var rad = 2.0 * Math.PI * freq;
			mDeltaRe = Math.Cos(rad);
			mDeltaIm = Math.Sin(rad);
			mOscRe = 0.707 * mDeltaRe - 0.707 * mDeltaIm;
			mOscIm = 0.707 * mDeltaIm + 0.707 * mDeltaRe;
			BaseLevel = Math.Sqrt(mRMS);
		}
	}
}
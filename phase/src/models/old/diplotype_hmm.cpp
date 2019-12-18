#include <models/diplotype_hmm.h>

diplotype_hmm::diplotype_hmm(bitmatrix & _H, hmm_parameters & _M, int _n_vars, int _n_haps) : H(_H), M(_M) {
	n_haps = _n_haps;
	n_vars = _n_vars;
	HET = vector < char > (n_vars, -1);
	ALT = vector < bool > (n_vars, false);

	//INIT EMIT0
	EMIT0[0][0] = M.ed; EMIT0[1][0] = M.ed; EMIT0[2][0] = M.ed;
	EMIT0[0][1] = M.ed; EMIT0[1][1] = M.ed; EMIT0[2][1] = M.ee;
	EMIT0[0][2] = M.ed; EMIT0[1][2] = M.ee; EMIT0[2][2] = M.ed;
	EMIT0[0][3] = M.ed; EMIT0[1][3] = M.ee; EMIT0[2][3] = M.ee;
	EMIT0[0][4] = M.ee; EMIT0[1][4] = M.ed; EMIT0[2][4] = M.ed;
	EMIT0[0][5] = M.ee; EMIT0[1][5] = M.ed; EMIT0[2][5] = M.ee;
	EMIT0[0][6] = M.ee; EMIT0[1][6] = M.ee; EMIT0[2][6] = M.ed;
	EMIT0[0][7] = M.ee; EMIT0[1][7] = M.ee; EMIT0[2][7] = M.ee;

	//INIT EMIT1
	EMIT1[0][0] = M.ee; EMIT1[1][0] = M.ee; EMIT1[2][0] = M.ee;
	EMIT1[0][1] = M.ee; EMIT1[1][1] = M.ee; EMIT1[2][1] = M.ed;
	EMIT1[0][2] = M.ee; EMIT1[1][2] = M.ed; EMIT1[2][2] = M.ee;
	EMIT1[0][3] = M.ee; EMIT1[1][3] = M.ed; EMIT1[2][3] = M.ed;
	EMIT1[0][4] = M.ed; EMIT1[1][4] = M.ee; EMIT1[2][4] = M.ee;
	EMIT1[0][5] = M.ed; EMIT1[1][5] = M.ee; EMIT1[2][5] = M.ed;
	EMIT1[0][6] = M.ed; EMIT1[1][6] = M.ed; EMIT1[2][6] = M.ee;
	EMIT1[0][7] = M.ed; EMIT1[1][7] = M.ed; EMIT1[2][7] = M.ed;
}

void diplotype_hmm::reallocate(vector < bool > & H0, vector < bool > & H1) {
	//SET HET AND ALT
	for (int l = 0, n_het = 0 ; l < n_vars ; l ++) {
		if (H0[l] != H1[l]) {
			HET[l] = (char)(n_het % 3);
			ALT[l] = false;
			n_het ++;
		} else {
			HET[l] = -1;
			ALT[l] = H0[l];
		}
	}

	//COMPUTE SEGMENTATION
	int nv = 0;
	segments = vector < int > ();
	for (int l = 0, n_hets = 0 ; l < n_vars ;) {
		n_hets += (HET[l] >= 0);
		if (n_hets == 4) {
			segments.push_back(nv);
			n_hets = 0;
			nv = 0;
		} else {
			nv ++;
			l++;
		}
	}
	segments.push_back(nv);

	// CHECK (TO BE REMOVED)
	int sumV = 0;
	for (int s = 0 ; s < segments.size() ; s ++) sumV += segments[s];
	assert(sumV == n_vars);
	//

	//REALLOCATE MEMORY
	if (segments.size() > Alpha.size()) {
		Alpha = vector < vector < double > > (segments.size(), vector < double > (HAP_NUMBER * n_haps, 0.0));
		Beta = vector < vector < double > > (segments.size(), vector < double > (HAP_NUMBER * n_haps, 0.0));
		AlphaSum = vector < vector < double > > (segments.size(), vector < double > (HAP_NUMBER, 0.0));
		BetaSum = vector < double > (HAP_NUMBER, 0.0);
	}
	if (prob1.size() == 0) {
		prob1 = vector < double > (HAP_NUMBER * n_haps, 1.0);
		prob2 = vector < double > (HAP_NUMBER * n_haps, 1.0);
		probSumH1 = vector < double > (HAP_NUMBER, 1.0);
		probSumH2 = vector < double > (HAP_NUMBER, 1.0);
		probSumK1 = vector < double > (n_haps, 1.0);
		probSumK2 = vector < double > (n_haps, 1.0);
		probSumT1 = 1.0;
		probSumT2 = 1.0;
	}
}

diplotype_hmm::~diplotype_hmm() {
}

void diplotype_hmm::forward() {
	curr_segment_index = 0;
	curr_segment_locus = 0;
	for (curr_locus = 0 ; curr_locus < n_vars ; curr_locus ++) {
		bool paired = (curr_locus % 2 == 0);

		paired?INIT2():INIT1();
		if (curr_locus != 0 && curr_segment_locus == 0) paired?COLLAPSE2(true):COLLAPSE1(true);
		if (curr_locus != 0 && curr_segment_locus != 0) paired?RUN2(true):RUN1(true);
		paired?SUM2():SUM1();

		if (curr_segment_locus == segments[curr_segment_index] - 1) paired?SUMK2():SUMK1();
		if (curr_segment_locus == segments[curr_segment_index] - 1) {
			Alpha[curr_segment_index] = (paired?prob2:prob1);
			AlphaSum[curr_segment_index] = (paired?probSumH2:probSumH1);
		}
		curr_segment_locus ++;
		if (curr_segment_locus >= segments[curr_segment_index]) {
			curr_segment_index++;
			curr_segment_locus = 0;
		}
	}
}

void diplotype_hmm::backward() {
	curr_segment_index = segments.size() - 1;
	curr_segment_locus = segments.back() - 1;
	for (curr_locus = n_vars - 1 ; curr_locus >= 0 ; curr_locus--) {
		bool paired = (curr_locus % 2 == 0);

		paired?INIT2():INIT1();
		if (curr_locus != n_vars - 1 && curr_segment_locus == segments[curr_segment_index] - 1) paired?COLLAPSE2(false):COLLAPSE1(false);
		if (curr_locus != n_vars - 1 && curr_segment_locus != segments[curr_segment_index] - 1) paired?RUN2(false):RUN1(false);
		paired?SUM2():SUM1();

		if (curr_segment_locus == 0) paired?SUMK2():SUMK1();
		if (curr_segment_locus == 0 && curr_locus != (n_vars - 1)) Beta[curr_segment_index] = (paired?prob2:prob1);
		if (curr_locus == 0) BetaSum=(paired?probSumH2:probSumH1);
		curr_segment_locus--;
		if (curr_segment_locus < 0 && curr_segment_index > 0) {
			curr_segment_index--;
			curr_segment_locus = segments[curr_segment_index] - 1;
		}
	}
}

void diplotype_hmm::sampleHaplotypes(vector < bool > & H0, vector < bool > & H1) {
	reallocate(H0, H1);
	forward();
	backward();

	vector < int > dip_sampled = vector < int > (segments.size(), -1);
	vector < double > dip_probs = vector < double > (HAP_NUMBER, 0.0);
	for (curr_segment_index = 0, curr_locus = 0 ; curr_segment_index < segments.size() ; curr_segment_index ++) {
		double sumHap = 0.0, sumDip = 0.0;
		if (curr_segment_index == 0) {
			for (int h = 0 ; h < HAP_NUMBER ; h ++) sumHap += BetaSum[h];
			for (int d = 0 ; d < HAP_NUMBER ; d ++) {
				dip_probs[d] = (BetaSum[d] / sumHap) * (BetaSum[HAP_NUMBER - d - 1] / sumHap);
				sumDip += dip_probs[d];
			}
		} else {
			TRANSH();
			for (int d = 0 ; d < HAP_NUMBER ; d ++) {
				int prev_h0 = dip_sampled[curr_segment_index - 1];
				int prev_h1 = HAP_NUMBER - dip_sampled[curr_segment_index - 1] - 1;
				dip_probs[d] = (HPROBS[prev_h0 * HAP_NUMBER + d] / sumHProbs) * (HPROBS[prev_h1 * HAP_NUMBER + (HAP_NUMBER - d - 1)] / sumHProbs);
				sumDip += dip_probs[d];
			}
		}
		dip_sampled[curr_segment_index] = rng.sample(dip_probs, sumDip);
		curr_locus += segments[curr_segment_index];
	}
	curr_segment_index = 0;
	curr_segment_locus = 0;
	for (curr_locus = 0  ; curr_locus < n_vars ; curr_locus ++) {
		if (HET[curr_locus] >= 0) {
			int idx_h0 = dip_sampled[curr_segment_index];
			int idx_h1 = HAP_NUMBER - dip_sampled[curr_segment_index] - 1;
			bool a0 = ALLELE(HAP_NUMBER - idx_h0 - 1, 2 - HET[curr_locus]);
			bool a1 = ALLELE(HAP_NUMBER - idx_h1 - 1, 2 - HET[curr_locus]);
			H0[curr_locus] = a0;
			H1[curr_locus] = a1;
		}
		curr_segment_locus ++;
		if (curr_segment_locus >= segments[curr_segment_index]) {
			curr_segment_index++;
			curr_segment_locus = 0;
		}
	}
}
// @DECOMP_OPT_FLAGS=-O1 -g2
// @DECOMP_IDO_VERSION=7.1
#include <PR/ultratypes.h>
#include <PR/ultraerror.h>
#include "libnaudio/n_synthInternals.h"

void n_alSynSetPan(N_ALVoice *v, u8 pan) {
	ALParam *update;

	if (v->pvoice) {
		/*
		 * get new update struct from the free list
		 */
		update = __n_allocParam();
		ALFailIf(update == 0, ERR_ALSYN_NO_UPDATE);

		/*
		 * set offset and pan data
		 */
		update->delta = n_syn->paramSamples + v->pvoice->offset;
		update->type = AL_FILTER_SET_PAN;
		update->data.i = pan;
		update->next = 0;

		n_alEnvmixerParam(v->pvoice, AL_FILTER_ADD_UPDATE, update);
	}
}

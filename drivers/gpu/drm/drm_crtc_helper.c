/*
 * Copyright (c) 2006-2008 Intel Corporation
 * Copyright (c) 2007 Dave Airlie <airlied@linux.ie>
 *
 * DRM core CRTC related functions
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * Authors:
 *      Keith Packard
 *	Eric Anholt <eric@anholt.net>
 *      Dave Airlie <airlied@linux.ie>
 *      Jesse Barnes <jesse.barnes@intel.com>
 */

#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/moduleparam.h>

#include <drm/drmP.h>
#include <drm/drm_atomic.h>
#include <drm/drm_crtc.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_plane_helper.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_edid.h>

/**
 * DOC: overview
 *
 * The CRTC modeset helper library provides a default set_config implementation
 * in drm_crtc_helper_set_config(). Plus a few other convenience functions using
 * the same callbacks which drivers can use to e.g. restore the modeset
 * configuration on resume with drm_helper_resume_force_mode().
 *
 * Note that this helper library doesn't track the current power state of CRTCs
 * and encoders. It can call callbacks like ->dpms() even though the hardware is
 * already in the desired state. This deficiency has been fixed in the atomic
 * helpers.
 *
 * The driver callbacks are mostly compatible with the atomic modeset helpers,
 * except for the handling of the primary plane: Atomic helpers require that the
 * primary plane is implemented as a real standalone plane and not directly tied
 * to the CRTC state. For easier transition this library provides functions to
 * implement the old semantics required by the CRTC helpers using the new plane
 * and atomic helper callbacks.
 *
 * Drivers are strongly urged to convert to the atomic helpers (by way of first
 * converting to the plane helpers). New drivers must not use these functions
 * but need to implement the atomic interface instead, potentially using the
 * atomic helpers for that.
 *
 * These legacy modeset helpers use the same function table structures as
 * all other modesetting helpers. See the documentation for struct
 * &drm_crtc_helper_funcs, struct &drm_encoder_helper_funcs and struct
 * &drm_connector_helper_funcs.
 */

/**
 * drm_helper_encoder_in_use - check if a given encoder is in use
 * @encoder: encoder to check
 *
 * Checks whether @encoder is with the current mode setting output configuration
 * in use by any connector. This doesn't mean that it is actually enabled since
 * the DPMS state is tracked separately.
 *
 * Returns:
 * True if @encoder is used, false otherwise.
 */
bool drm_helper_encoder_in_use(struct drm_encoder *encoder)
{
	struct drm_connector *connector;
	struct drm_device *dev = encoder->dev;

	/*
	 * We can expect this mutex to be locked if we are not panicking.
	 * Locking is currently fubar in the panic handler.
	 */
	if (!oops_in_progress) {
		WARN_ON(!mutex_is_locked(&dev->mode_config.mutex));
		WARN_ON(!drm_modeset_is_locked(&dev->mode_config.connection_mutex));
	}

	drm_for_each_connector(connector, dev)
		if (connector->encoder == encoder)
			return true;
	return false;
}
EXPORT_SYMBOL(drm_helper_encoder_in_use);

/**
 * drm_helper_crtc_in_use - check if a given CRTC is in a mode_config
 * @crtc: CRTC to check
 *
 * Checks whether @crtc is with the current mode setting output configuration
 * in use by any connector. This doesn't mean that it is actually enabled since
 * the DPMS state is tracked separately.
 *
 * Returns:
 * True if @crtc is used, false otherwise.
 */
bool drm_helper_crtc_in_use(struct drm_crtc *crtc)
{
	struct drm_encoder *encoder;
	struct drm_device *dev = crtc->dev;

	/*
	 * We can expect this mutex to be locked if we are not panicking.
	 * Locking is currently fubar in the panic handler.
	 */
	if (!oops_in_progress)
		WARN_ON(!mutex_is_locked(&dev->mode_config.mutex));

	drm_for_each_encoder(encoder, dev)
		if (encoder->crtc == crtc && drm_helper_encoder_in_use(encoder))
			return true;
	return false;
}
EXPORT_SYMBOL(drm_helper_crtc_in_use);

static void
drm_encoder_disable(struct drm_encoder *encoder)
{
	const struct drm_encoder_helper_funcs *encoder_funcs = encoder->helper_private;

	if (!encoder_funcs)
		return;

	drm_bridge_disable(encoder->bridge);

	if (encoder_funcs->disable)
		(*encoder_funcs->disable)(encoder);
	else if (encoder_funcs->dpms)
		(*encoder_funcs->dpms)(encoder, DRM_MODE_DPMS_OFF);

	drm_bridge_post_disable(encoder->bridge);
}

static void __drm_helper_disable_unused_functions(struct drm_device *dev)
{
	struct drm_encoder *encoder;
	struct drm_crtc *crtc;

	drm_warn_on_modeset_not_all_locked(dev);

	drm_for_each_encoder(encoder, dev) {
		if (!drm_helper_encoder_in_use(encoder)) {
			drm_encoder_disable(encoder);
			/* disconnect encoder from any connector */
			encoder->crtc = NULL;
		}
	}

	drm_for_each_crtc(crtc, dev) {
		const struct drm_crtc_helper_funcs *crtc_funcs = crtc->helper_private;
		crtc->enabled = drm_helper_crtc_in_use(crtc);
		if (!crtc->enabled) {
			if (crtc_funcs->disable)
				(*crtc_funcs->disable)(crtc);
			else
				(*crtc_funcs->dpms)(crtc, DRM_MODE_DPMS_OFF);
			crtc->primary->fb = NULL;
		}
	}
}

/**
 * drm_helper_disable_unused_functions - disable unused objects
 * @dev: DRM device
 *
 * This function walks through the entire mode setting configuration of @dev. It
 * will remove any CRTC links of unused encoders and encoder links of
 * disconnected connectors. Then it will disable all unused encoders and CRTCs
 * either by calling their disable callback if available or by calling their
 * dpms callback with DRM_MODE_DPMS_OFF.
 *
 * NOTE:
 *
 * This function is part of the legacy modeset helper library and will cause
 * major confusion with atomic drivers. This is because atomic helpers guarantee
 * to never call ->disable() hooks on a disabled function, or ->enable() hooks
 * on an enabled functions. drm_helper_disable_unused_functions() on the other
 * hand throws such guarantees into the wind and calls disable hooks
 * unconditionally on unused functions.
 */
void drm_helper_disable_unused_functions(struct drm_device *dev)
{
	if (drm_core_check_feature(dev, DRIVER_ATOMIC))
		DRM_ERROR("Called for atomic driver, this is not what you want.\n");

	drm_modeset_lock_all(dev);
	__drm_helper_disable_unused_functions(dev);
	drm_modeset_unlock_all(dev);
}
EXPORT_SYMBOL(drm_helper_disable_unused_functions);

/*
 * Check the CRTC we're going to map each output to vs. its current
 * CRTC.  If they don't match, we have to disable the output and the CRTC
 * since the driver will have to re-route things.
 */
static void
drm_crtc_prepare_encoders(struct drm_device *dev)
{
	const struct drm_encoder_helper_funcs *encoder_funcs;
	struct drm_encoder *encoder;

	drm_for_each_encoder(encoder, dev) {
		encoder_funcs = encoder->helper_private;
		if (!encoder_funcs)
			continue;

		/* Disable unused encoders */
		if (encoder->crtc == NULL)
			drm_encoder_disable(encoder);
		/* Disable encoders whose CRTC is about to change */
		if (encoder_funcs->get_crtc &&
		    encoder->crtc != (*encoder_funcs->get_crtc)(encoder))
			drm_encoder_disable(encoder);
	}
}

/**
 * drm_crtc_helper_set_mode - internal helper to set a mode
 * @crtc: CRTC to program
 * @mode: mode to use
 * @x: horizontal offset into the surface
 * @y: vertical offset into the surface
 * @old_fb: old framebuffer, for cleanup
 *
 * Try to set @mode on @crtc.  Give @crtc and its associated connectors a chance
 * to fixup or reject the mode prior to trying to set it. This is an internal
 * helper that drivers could e.g. use to update properties that require the
 * entire output pipe to be disabled and re-enabled in a new configuration. For
 * example for changing whether audio is enabled on a hdmi link or for changing
 * panel fitter or dither attributes. It is also called by the
 * drm_crtc_helper_set_config() helper function to drive the mode setting
 * sequence.
 *
 * Returns:
 * True if the mode was set successfully, false otherwise.
 */
bool drm_crtc_helper_set_mode(struct drm_crtc *crtc,
			      struct drm_display_mode *mode,
			      int x, int y,
			      struct drm_framebuffer *old_fb)
{
	struct drm_device *dev = crtc->dev;
	struct drm_display_mode *adjusted_mode, saved_mode, saved_hwmode;
	const struct drm_crtc_helper_funcs *crtc_funcs = crtc->helper_private;
	const struct drm_encoder_helper_funcs *encoder_funcs;
	int saved_x, saved_y;
	bool saved_enabled;
	struct drm_encoder *encoder;
	bool ret = true;

	drm_warn_on_modeset_not_all_locked(dev);

	saved_enabled = crtc->enabled;
	crtc->enabled = drm_helper_crtc_in_use(crtc);
	if (!crtc->enabled)
		return true;

	adjusted_mode = drm_mode_duplicate(dev, mode);
	if (!adjusted_mode) {
		crtc->enabled = saved_enabled;
		return false;
	}

	saved_mode = crtc->mode;
	saved_hwmode = crtc->hwmode;
	saved_x = crtc->x;
	saved_y = crtc->y;

	/* Update crtc values up front so the driver can rely on them for mode
	 * setting.
	 */
	crtc->mode = *mode;
	crtc->x = x;
	crtc->y = y;

	/* Pass our mode to the connectors and the CRTC to give them a chance to
	 * adjust it according to limitations or connector properties, and also
	 * a chance to reject the mode entirely.
	 */
	drm_for_each_encoder(encoder, dev) {

		if (encoder->crtc != crtc)
			continue;

		encoder_funcs = encoder->helper_private;
		if (!encoder_funcs)
			continue;

		ret = drm_bridge_mode_fixup(encoder->bridge,
			mode, adjusted_mode);
		if (!ret) {
			DRM_DEBUG_KMS("Bridge fixup failed\n");
			goto done;
		}

		encoder_funcs = encoder->helper_private;
		if (encoder_funcs->mode_fixup) {
			if (!(ret = encoder_funcs->mode_fixup(encoder, mode,
							      adjusted_mode))) {
				DRM_DEBUG_KMS("Encoder fixup failed\n");
				goto done;
			}
		}
	}

	if (crtc_funcs->mode_fixup) {
		if (!(ret = crtc_funcs->mode_fixup(crtc, mode,
						adjusted_mode))) {
			DRM_DEBUG_KMS("CRTC fixup failed\n");
			goto done;
		}
	}
	DRM_DEBUG_KMS("[CRTC:%d:%s]\n", crtc->base.id, crtc->name);

	crtc->hwmode = *adjusted_mode;

	/* Prepare the encoders and CRTCs before setting the mode. */
	drm_for_each_encoder(encoder, dev) {

		if (encoder->crtc != crtc)
			continue;

		encoder_funcs = encoder->helper_private;
		if (!encoder_funcs)
			continue;

		drm_bridge_disable(encoder->bridge);

		/* Disable the encoders as the first thing we do. */
		if (encoder_funcs->prepare)
			encoder_funcs->prepare(encoder);

		drm_bridge_post_disable(encoder->bridge);
	}

	drm_crtc_prepare_encoders(dev);

	crtc_funcs->prepare(crtc);

	/* Set up the DPLL and any encoders state that needs to adjust or depend
	 * on the DPLL.
	 */
	ret = !crtc_funcs->mode_set(crtc, mode, adjusted_mode, x, y, old_fb);
	if (!ret)
	    goto done;

	drm_for_each_encoder(encoder, dev) {

		if (encoder->crtc != crtc)
			continue;

		encoder_funcs = encoder->helper_private;
		if (!encoder_funcs)
			continue;

		DRM_DEBUG_KMS("[ENCODER:%d:%s] set [MODE:%d:%s]\n",
			encoder->base.id, encoder->name,
			mode->base.id, mode->name);
		if (encoder_funcs->mode_set)
			encoder_funcs->mode_set(encoder, mode, adjusted_mode);

		drm_bridge_mode_set(encoder->bridge, mode, adjusted_mode);
	}

	/* Now enable the clocks, plane, pipe, and connectors that we set up. */
	crtc_funcs->commit(crtc);

	drm_for_each_encoder(encoder, dev) {

		if (encoder->crtc != crtc)
			continue;

		encoder_funcs = encoder->helper_private;
		if (!encoder_funcs)
			continue;

		drm_bridge_pre_enable(encoder->bridge);

		if (encoder_funcs->commit)
			encoder_funcs->commit(encoder);

		drm_bridge_enable(encoder->bridge);
	}

	/* Calculate and store various constants which
	 * are later needed by vblank and swap-completion
	 * timestamping. They are derived from true hwmode.
	 */
	drm_calc_timestamping_constants(crtc, &crtc->hwmode);

	/* FIXME: add subpixel order */
done:
	drm_mode_destroy(dev, adjusted_mode);
	if (!ret) {
		crtc->enabled = saved_enabled;
		crtc->mode = saved_mode;
		crtc->hwmode = saved_hwmode;
		crtc->x = saved_x;
		crtc->y = saved_y;
	}

	return ret;
}
EXPORT_SYMBOL(drm_crtc_helper_set_mode);

static void
drm_crtc_helper_disable(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	struct drm_connector *connector;
	struct drm_encoder *encoder;

	/* Decouple all encoders and their attached connectors from this crtc */
	drm_for_each_encoder(encoder, dev) {
		if (encoder->crtc != crtc)
			continue;

		drm_for_each_connector(connector, dev) {
			if (connector->encoder != encoder)
				continue;

			connector->encoder = NULL;

			/*
			 * drm_helper_disable_unused_functions() ought to be
			 * doing this, but since we've decoupled the encoder
			 * from the connector above, the required connection
			 * between them is henceforth no longer available.
			 */
			connector->dpms = DRM_MODE_DPMS_OFF;

			/* we keep a reference while the encoder is bound */
			drm_connector_unreference(connector);
		}
	}

	__drm_helper_disable_unused_functions(dev);
}

/**
 * drm_crtc_helper_set_config - set a new config from userspace
 * @set: mode set configuration
 *
 * The drm_crtc_helper_set_config() helper function implements the set_config
 * callback of struct &drm_crtc_funcs for drivers using the legacy CRTC helpers.
 *
 * It first tries to locate the best encoder for each connector by calling the
 * connector ->best_encoder() (struct &drm_connector_helper_funcs) helper
 * operation.
 *
 * After locating the appropriate encoders, the helper function will call the
 * mode_fixup encoder and CRTC helper operations to adjust the requested mode,
 * or reject it completely in which case an error will be returned to the
 * application. If the new configuration after mode adjustment is identical to
 * the current configuration the helper function will return without performing
 * any other operation.
 *
 * If the adjusted mode is identical to the current mode but changes to the
 * frame buffer need to be applied, the drm_crtc_helper_set_config() function
 * will call the CRTC ->mode_set_base() (struct &drm_crtc_helper_funcs) helper
 * operation.
 *
 * If the adjusted mode differs from the current mode, or if the
 * ->mode_set_base() helper operation is not provided, the helper function
 * performs a full mode set sequence by calling the ->prepare(), ->mode_set()
 * and ->commit() CRTC and encoder helper operations, in that order.
 * Alternatively it can also use the dpms and disable helper operations. For
 * details see struct &drm_crtc_helper_funcs and struct
 * &drm_encoder_helper_funcs.
 *
 * This function is deprecated.  New drivers must implement atomic modeset
 * support, for which this function is unsuitable. Instead drivers should use
 * drm_atomic_helper_set_config().
 *
 * Returns:
 * Returns 0 on success, negative errno numbers on failure.
 */
int drm_crtc_helper_set_config(struct drm_mode_set *set)
{
	struct drm_device *dev;
	struct drm_crtc **save_encoder_crtcs, *new_crtc;
	struct drm_encoder **save_connector_encoders, *new_encoder, *encoder;
	bool mode_changed = false; /* if true do a full mode set */
	bool fb_changed = false; /* if true and !mode_changed just do a flip */
	struct drm_connector *connector;
	int count = 0, ro, fail = 0;
	const struct drm_crtc_helper_funcs *crtc_funcs;
	struct drm_mode_set save_set;
	int ret;
	int i;

	DRM_DEBUG_KMS("\n");

	BUG_ON(!set);
	BUG_ON(!set->crtc);
	BUG_ON(!set->crtc->helper_private);

	/* Enforce sane interface api - has been abused by the fb helper. */
	BUG_ON(!set->mode && set->fb);
	BUG_ON(set->fb && set->num_connectors == 0);

	crtc_funcs = set->crtc->helper_private;

	if (!set->mode)
		set->fb = NULL;

	if (set->fb) {
		DRM_DEBUG_KMS("[CRTC:%d:%s] [FB:%d] #connectors=%d (x y) (%i %i)\n",
			      set->crtc->base.id, set->crtc->name,
			      set->fb->base.id,
			      (int)set->num_connectors, set->x, set->y);
	} else {
		DRM_DEBUG_KMS("[CRTC:%d:%s] [NOFB]\n",
			      set->crtc->base.id, set->crtc->name);
		drm_crtc_helper_disable(set->crtc);
		return 0;
	}

	dev = set->crtc->dev;

	drm_warn_on_modeset_not_all_locked(dev);

	/*
	 * Allocate space for the backup of all (non-pointer) encoder and
	 * connector data.
	 */
	save_encoder_crtcs = kzalloc(dev->mode_config.num_encoder *
				sizeof(struct drm_crtc *), GFP_KERNEL);
	if (!save_encoder_crtcs)
		return -ENOMEM;

	save_connector_encoders = kzalloc(dev->mode_config.num_connector *
				sizeof(struct drm_encoder *), GFP_KERNEL);
	if (!save_connector_encoders) {
		kfree(save_encoder_crtcs);
		return -ENOMEM;
	}

	/*
	 * Copy data. Note that driver private data is not affected.
	 * Should anything bad happen only the expected state is
	 * restored, not the drivers personal bookkeeping.
	 */
	count = 0;
	drm_for_each_encoder(encoder, dev) {
		save_encoder_crtcs[count++] = encoder->crtc;
	}

	count = 0;
	drm_for_each_connector(connector, dev) {
		save_connector_encoders[count++] = connector->encoder;
	}

	save_set.crtc = set->crtc;
	save_set.mode = &set->crtc->mode;
	save_set.x = set->crtc->x;
	save_set.y = set->crtc->y;
	save_set.fb = set->crtc->primary->fb;

	/* We should be able to check here if the fb has the same properties
	 * and then just flip_or_move it */
	if (set->crtc->primary->fb != set->fb) {
		/* If we have no fb then treat it as a full mode set */
		if (set->crtc->primary->fb == NULL) {
			DRM_DEBUG_KMS("crtc has no fb, full mode set\n");
			mode_changed = true;
		} else if (set->fb->pixel_format !=
			   set->crtc->primary->fb->pixel_format) {
			mode_changed = true;
		} else
			fb_changed = true;
	}

	if (set->x != set->crtc->x || set->y != set->crtc->y)
		fb_changed = true;

	if (!drm_mode_equal(set->mode, &set->crtc->mode)) {
		DRM_DEBUG_KMS("modes are different, full mode set\n");
		drm_mode_debug_printmodeline(&set->crtc->mode);
		drm_mode_debug_printmodeline(set->mode);
		mode_changed = true;
	}

	/* take a reference on all unbound connectors in set, reuse the
	 * already taken reference for bound connectors
	 */
	for (ro = 0; ro < set->num_connectors; ro++) {
		if (set->connectors[ro]->encoder)
			continue;
		drm_connector_reference(set->connectors[ro]);
	}

	/* a) traverse passed in connector list and get encoders for them */
	count = 0;
	drm_for_each_connector(connector, dev) {
		const struct drm_connector_helper_funcs *connector_funcs =
			connector->helper_private;
		new_encoder = connector->encoder;
		for (ro = 0; ro < set->num_connectors; ro++) {
			if (set->connectors[ro] == connector) {
				new_encoder = connector_funcs->best_encoder(connector);
				/* if we can't get an encoder for a connector
				   we are setting now - then fail */
				if (new_encoder == NULL)
					/* don't break so fail path works correct */
					fail = 1;

				if (connector->dpms != DRM_MODE_DPMS_ON) {
					DRM_DEBUG_KMS("connector dpms not on, full mode switch\n");
					mode_changed = true;
				}

				break;
			}
		}

		if (new_encoder != connector->encoder) {
			DRM_DEBUG_KMS("encoder changed, full mode switch\n");
			mode_changed = true;
			/* If the encoder is reused for another connector, then
			 * the appropriate crtc will be set later.
			 */
			if (connector->encoder)
				connector->encoder->crtc = NULL;
			connector->encoder = new_encoder;
		}
	}

	if (fail) {
		ret = -EINVAL;
		goto fail;
	}

	count = 0;
	drm_for_each_connector(connector, dev) {
		if (!connector->encoder)
			continue;

		if (connector->encoder->crtc == set->crtc)
			new_crtc = NULL;
		else
			new_crtc = connector->encoder->crtc;

		for (ro = 0; ro < set->num_connectors; ro++) {
			if (set->connectors[ro] == connector)
				new_crtc = set->crtc;
		}

		/* Make sure the new CRTC will work with the encoder */
		if (new_crtc &&
		    !drm_encoder_crtc_ok(connector->encoder, new_crtc)) {
			ret = -EINVAL;
			goto fail;
		}
		if (new_crtc != connector->encoder->crtc) {
			DRM_DEBUG_KMS("crtc changed, full mode switch\n");
			mode_changed = true;
			connector->encoder->crtc = new_crtc;
		}
		if (new_crtc) {
			DRM_DEBUG_KMS("[CONNECTOR:%d:%s] to [CRTC:%d:%s]\n",
				      connector->base.id, connector->name,
				      new_crtc->base.id, new_crtc->name);
		} else {
			DRM_DEBUG_KMS("[CONNECTOR:%d:%s] to [NOCRTC]\n",
				      connector->base.id, connector->name);
		}
	}

	/* mode_set_base is not a required function */
	if (fb_changed && !crtc_funcs->mode_set_base)
		mode_changed = true;

	if (mode_changed) {
		if (drm_helper_crtc_in_use(set->crtc)) {
			DRM_DEBUG_KMS("attempting to set mode from"
					" userspace\n");
			drm_mode_debug_printmodeline(set->mode);
			set->crtc->primary->fb = set->fb;
			if (!drm_crtc_helper_set_mode(set->crtc, set->mode,
						      set->x, set->y,
						      save_set.fb)) {
				DRM_ERROR("failed to set mode on [CRTC:%d:%s]\n",
					  set->crtc->base.id, set->crtc->name);
				set->crtc->primary->fb = save_set.fb;
				ret = -EINVAL;
				goto fail;
			}
			DRM_DEBUG_KMS("Setting connector DPMS state to on\n");
			for (i = 0; i < set->num_connectors; i++) {
				DRM_DEBUG_KMS("\t[CONNECTOR:%d:%s] set DPMS on\n", set->connectors[i]->base.id,
					      set->connectors[i]->name);
				set->connectors[i]->funcs->dpms(set->connectors[i], DRM_MODE_DPMS_ON);
			}
		}
		__drm_helper_disable_unused_functions(dev);
	} else if (fb_changed) {
		set->crtc->x = set->x;
		set->crtc->y = set->y;
		set->crtc->primary->fb = set->fb;
		ret = crtc_funcs->mode_set_base(set->crtc,
						set->x, set->y, save_set.fb);
		if (ret != 0) {
			set->crtc->x = save_set.x;
			set->crtc->y = save_set.y;
			set->crtc->primary->fb = save_set.fb;
			goto fail;
		}
	}

	kfree(save_connector_encoders);
	kfree(save_encoder_crtcs);
	return 0;

fail:
	/* Restore all previous data. */
	count = 0;
	drm_for_each_encoder(encoder, dev) {
		encoder->crtc = save_encoder_crtcs[count++];
	}

	count = 0;
	drm_for_each_connector(connector, dev) {
		connector->encoder = save_connector_encoders[count++];
	}

	/* after fail drop reference on all unbound connectors in set, let
	 * bound connectors keep their reference
	 */
	for (ro = 0; ro < set->num_connectors; ro++) {
		if (set->connectors[ro]->encoder)
			continue;
		drm_connector_unreference(set->connectors[ro]);
	}

	/* Try to restore the config */
	if (mode_changed &&
	    !drm_crtc_helper_set_mode(save_set.crtc, save_set.mode, save_set.x,
				      save_set.y, save_set.fb))
		DRM_ERROR("failed to restore config after modeset failure\n");

	kfree(save_connector_encoders);
	kfree(save_encoder_crtcs);
	return ret;
}
EXPORT_SYMBOL(drm_crtc_helper_set_config);

static int drm_helper_choose_encoder_dpms(struct drm_encoder *encoder)
{
	int dpms = DRM_MODE_DPMS_OFF;
	struct drm_connector *connector;
	struct drm_device *dev = encoder->dev;

	drm_for_each_connector(connector, dev)
		if (connector->encoder == encoder)
			if (connector->dpms < dpms)
				dpms = connector->dpms;
	return dpms;
}

/* Helper which handles bridge ordering around encoder dpms */
static void drm_helper_encoder_dpms(struct drm_encoder *encoder, int mode)
{
	struct drm_bridge *bridge = encoder->bridge;
	const struct drm_encoder_helper_funcs *encoder_funcs;

	encoder_funcs = encoder->helper_private;
	if (!encoder_funcs)
		return;

	if (mode == DRM_MODE_DPMS_ON)
		drm_bridge_pre_enable(bridge);
	else
		drm_bridge_disable(bridge);

	if (encoder_funcs->dpms)
		encoder_funcs->dpms(encoder, mode);

	if (mode == DRM_MODE_DPMS_ON)
		drm_bridge_enable(bridge);
	else
		drm_bridge_post_disable(bridge);
}

static int drm_helper_choose_crtc_dpms(struct drm_crtc *crtc)
{
	int dpms = DRM_MODE_DPMS_OFF;
	struct drm_connector *connector;
	struct drm_device *dev = crtc->dev;

	drm_for_each_connector(connector, dev)
		if (connector->encoder && connector->encoder->crtc == crtc)
			if (connector->dpms < dpms)
				dpms = connector->dpms;
	return dpms;
}

#ifdef CONFIG_CPU_LOONGSON3
void turn_off_lvds(void);
void turn_on_lvds(void);
#endif
/**
 * drm_helper_connector_dpms() - connector dpms helper implementation
 * @connector: affected connector
 * @mode: DPMS mode
 *
 * The drm_helper_connector_dpms() helper function implements the ->dpms()
 * callback of struct &drm_connector_funcs for drivers using the legacy CRTC helpers.
 *
 * This is the main helper function provided by the CRTC helper framework for
 * implementing the DPMS connector attribute. It computes the new desired DPMS
 * state for all encoders and CRTCs in the output mesh and calls the ->dpms()
 * callbacks provided by the driver in struct &drm_crtc_helper_funcs and struct
 * &drm_encoder_helper_funcs appropriately.
 *
 * This function is deprecated.  New drivers must implement atomic modeset
 * support, for which this function is unsuitable. Instead drivers should use
 * drm_atomic_helper_connector_dpms().
 *
 * Returns:
 * Always returns 0.
 */
int drm_helper_connector_dpms(struct drm_connector *connector, int mode)
{
	struct drm_encoder *encoder = connector->encoder;
	struct drm_crtc *crtc = encoder ? encoder->crtc : NULL;
	int old_dpms, encoder_dpms = DRM_MODE_DPMS_OFF;

	if (mode == connector->dpms)
		return 0;

	old_dpms = connector->dpms;
	connector->dpms = mode;

	if (encoder)
		encoder_dpms = drm_helper_choose_encoder_dpms(encoder);

	/* from off to on, do crtc then encoder */
	if (mode < old_dpms) {
#ifdef CONFIG_CPU_LOONGSON3
		if(connector->connector_type == DRM_MODE_CONNECTOR_LVDS)
			turn_on_lvds();
#endif
		if (crtc) {
			const struct drm_crtc_helper_funcs *crtc_funcs = crtc->helper_private;
			if (crtc_funcs->dpms)
				(*crtc_funcs->dpms) (crtc,
						     drm_helper_choose_crtc_dpms(crtc));
		}
		if (encoder)
			drm_helper_encoder_dpms(encoder, encoder_dpms);
	}

	/* from on to off, do encoder then crtc */
	if (mode > old_dpms) {
#ifdef CONFIG_CPU_LOONGSON3
		if(connector->connector_type == DRM_MODE_CONNECTOR_LVDS)
			turn_off_lvds();
#endif
		if (encoder)
			drm_helper_encoder_dpms(encoder, encoder_dpms);
		if (crtc) {
			const struct drm_crtc_helper_funcs *crtc_funcs = crtc->helper_private;
			if (crtc_funcs->dpms)
				(*crtc_funcs->dpms) (crtc,
						     drm_helper_choose_crtc_dpms(crtc));
		}
	}

	return 0;
}
EXPORT_SYMBOL(drm_helper_connector_dpms);

/**
 * drm_helper_resume_force_mode - force-restore mode setting configuration
 * @dev: drm_device which should be restored
 *
 * Drivers which use the mode setting helpers can use this function to
 * force-restore the mode setting configuration e.g. on resume or when something
 * else might have trampled over the hw state (like some overzealous old BIOSen
 * tended to do).
 *
 * This helper doesn't provide a error return value since restoring the old
 * config should never fail due to resource allocation issues since the driver
 * has successfully set the restored configuration already. Hence this should
 * boil down to the equivalent of a few dpms on calls, which also don't provide
 * an error code.
 *
 * Drivers where simply restoring an old configuration again might fail (e.g.
 * due to slight differences in allocating shared resources when the
 * configuration is restored in a different order than when userspace set it up)
 * need to use their own restore logic.
 *
 * This function is deprecated. New drivers should implement atomic mode-
 * setting and use the atomic suspend/resume helpers.
 *
 * See also:
 * drm_atomic_helper_suspend(), drm_atomic_helper_resume()
 */
void drm_helper_resume_force_mode(struct drm_device *dev)
{
	struct drm_crtc *crtc;
	struct drm_encoder *encoder;
	const struct drm_crtc_helper_funcs *crtc_funcs;
	int encoder_dpms;
	bool ret;

	drm_modeset_lock_all(dev);
	drm_for_each_crtc(crtc, dev) {

		if (!crtc->enabled)
			continue;

		ret = drm_crtc_helper_set_mode(crtc, &crtc->mode,
					       crtc->x, crtc->y, crtc->primary->fb);

		/* Restoring the old config should never fail! */
		if (ret == false)
			DRM_ERROR("failed to set mode on crtc %p\n", crtc);

		/* Turn off outputs that were already powered off */
		if (drm_helper_choose_crtc_dpms(crtc)) {
			drm_for_each_encoder(encoder, dev) {

				if(encoder->crtc != crtc)
					continue;

				encoder_dpms = drm_helper_choose_encoder_dpms(
							encoder);

				drm_helper_encoder_dpms(encoder, encoder_dpms);
			}

			crtc_funcs = crtc->helper_private;
			if (crtc_funcs->dpms)
				(*crtc_funcs->dpms) (crtc,
						     drm_helper_choose_crtc_dpms(crtc));
		}
	}

	/* disable the unused connectors while restoring the modesetting */
	__drm_helper_disable_unused_functions(dev);
	drm_modeset_unlock_all(dev);
}
EXPORT_SYMBOL(drm_helper_resume_force_mode);

/**
 * drm_helper_crtc_mode_set - mode_set implementation for atomic plane helpers
 * @crtc: DRM CRTC
 * @mode: DRM display mode which userspace requested
 * @adjusted_mode: DRM display mode adjusted by ->mode_fixup callbacks
 * @x: x offset of the CRTC scanout area on the underlying framebuffer
 * @y: y offset of the CRTC scanout area on the underlying framebuffer
 * @old_fb: previous framebuffer
 *
 * This function implements a callback useable as the ->mode_set callback
 * required by the CRTC helpers. Besides the atomic plane helper functions for
 * the primary plane the driver must also provide the ->mode_set_nofb callback
 * to set up the CRTC.
 *
 * This is a transitional helper useful for converting drivers to the atomic
 * interfaces.
 */
int drm_helper_crtc_mode_set(struct drm_crtc *crtc, struct drm_display_mode *mode,
			     struct drm_display_mode *adjusted_mode, int x, int y,
			     struct drm_framebuffer *old_fb)
{
	struct drm_crtc_state *crtc_state;
	const struct drm_crtc_helper_funcs *crtc_funcs = crtc->helper_private;
	int ret;

	if (crtc->funcs->atomic_duplicate_state)
		crtc_state = crtc->funcs->atomic_duplicate_state(crtc);
	else {
		if (!crtc->state)
			drm_atomic_helper_crtc_reset(crtc);

		crtc_state = drm_atomic_helper_crtc_duplicate_state(crtc);
	}

	if (!crtc_state)
		return -ENOMEM;

	crtc_state->planes_changed = true;
	crtc_state->mode_changed = true;
	ret = drm_atomic_set_mode_for_crtc(crtc_state, mode);
	if (ret)
		goto out;
	drm_mode_copy(&crtc_state->adjusted_mode, adjusted_mode);

	if (crtc_funcs->atomic_check) {
		ret = crtc_funcs->atomic_check(crtc, crtc_state);
		if (ret)
			goto out;
	}

	swap(crtc->state, crtc_state);

	crtc_funcs->mode_set_nofb(crtc);

	ret = drm_helper_crtc_mode_set_base(crtc, x, y, old_fb);

out:
	if (crtc_state) {
		if (crtc->funcs->atomic_destroy_state)
			crtc->funcs->atomic_destroy_state(crtc, crtc_state);
		else
			drm_atomic_helper_crtc_destroy_state(crtc, crtc_state);
	}

	return ret;
}
EXPORT_SYMBOL(drm_helper_crtc_mode_set);

/**
 * drm_helper_crtc_mode_set_base - mode_set_base implementation for atomic plane helpers
 * @crtc: DRM CRTC
 * @x: x offset of the CRTC scanout area on the underlying framebuffer
 * @y: y offset of the CRTC scanout area on the underlying framebuffer
 * @old_fb: previous framebuffer
 *
 * This function implements a callback useable as the ->mode_set_base used
 * required by the CRTC helpers. The driver must provide the atomic plane helper
 * functions for the primary plane.
 *
 * This is a transitional helper useful for converting drivers to the atomic
 * interfaces.
 */
int drm_helper_crtc_mode_set_base(struct drm_crtc *crtc, int x, int y,
				  struct drm_framebuffer *old_fb)
{
	struct drm_plane_state *plane_state;
	struct drm_plane *plane = crtc->primary;

	if (plane->funcs->atomic_duplicate_state)
		plane_state = plane->funcs->atomic_duplicate_state(plane);
	else {
		if (!plane->state)
			drm_atomic_helper_plane_reset(plane);

		plane_state = drm_atomic_helper_plane_duplicate_state(plane);
	}
	if (!plane_state)
		return -ENOMEM;
	plane_state->plane = plane;

	plane_state->crtc = crtc;
	drm_atomic_set_fb_for_plane(plane_state, crtc->primary->fb);
	plane_state->crtc_x = 0;
	plane_state->crtc_y = 0;
	plane_state->crtc_h = crtc->mode.vdisplay;
	plane_state->crtc_w = crtc->mode.hdisplay;
	plane_state->src_x = x << 16;
	plane_state->src_y = y << 16;
	plane_state->src_h = crtc->mode.vdisplay << 16;
	plane_state->src_w = crtc->mode.hdisplay << 16;

	return drm_plane_helper_commit(plane, plane_state, old_fb);
}
EXPORT_SYMBOL(drm_helper_crtc_mode_set_base);

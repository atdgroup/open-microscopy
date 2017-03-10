
void upix_camera_release_lock(Upix1311Camera* camera);
int upix_get_serial_number(Upix1311Camera* upix_camera, BYTE *serial_number);
int get_real_exposure_from_upix_exposure(Upix1311Camera *upix_camera, int upix_exposure, int *real_exposure);
int get_upix_exposure_from_real_exposure(Upix1311Camera *upix_camera, int real_exposure, int *upix_exposure);
int get_exposure(Upix1311Camera *upix_camera, double *exposure);
int set_exposure(Upix1311Camera *upix_camera, int exposure);
int set_gain(Upix1311Camera *upix_camera, int gain);
int get_gain(Upix1311Camera *upix_camera, double *gain);
int upix_initialise (Upix1311Camera* upix_camera);
int upix_uninitialise (Upix1311Camera* camera);
int camera_play(Upix1311Camera* camera, DS_SNAP_MODE SnapMode);
int camera_stop(Upix1311Camera* camera);
int snap_image (Upix1311Camera* camera);
FIBITMAP* get_image(Upix1311Camera* camera);
int start_live(Upix1311Camera* camera);
int set_power(Upix1311Camera* camera, int power);
int initialise (Upix1311Camera* camera);


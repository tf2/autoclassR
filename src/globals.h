
/************************ global externs ***********************************/
/*  22oct94 wmt: create */

extern void ***G_plist;
extern shortstr G_transforms[], G_att_type_data[];

extern int G_db_length, G_m_id, G_m_length;
extern int G_plength;
extern clsf_DS G_clsf_store;
extern fxlstr G_checkpoint_file;
extern time_t G_search_cycle_begin_time, G_last_checkpoint_written, G_min_checkpoint_period;
extern database_DS G_input_data_base, *G_db_list;
extern model_DS *G_model_list;
extern int G_break_on_warnings;
extern float G_likelihood_tolerance_ratio;
extern unsigned int G_save_compact_p;
extern shortstr G_ac_version;
extern FILE *G_log_file_fp, *G_stream;
extern char G_absolute_pathname[];
extern int G_line_cnt_max;
/* only supported under unix, since it does system calls to mv and rm */
extern int G_safe_file_writing_p;
extern char G_data_file_format[];
extern int G_solaris;
extern double G_rand_base_normalizer;
extern clsf_DS G_training_clsf;
extern int G_prediction_p;
extern int G_interactive_p;
extern int G_num_cycles;
extern char G_slash;

/* for debugging */
extern int G_clsf_storage_log_p, G_n_freed_classes, G_n_create_classes_after_free;

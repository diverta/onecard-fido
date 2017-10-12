/*
 * card_manager.h
 *
 *  Created on: 2016/05/21
 *      Author: r.suzuki
 */

#ifndef CARD_MANAGER_H_
#define CARD_MANAGER_H_

/*******************************************************************************
 * include.
 ******************************************************************************/
#include <stdint.h>


/*******************************************************************************
 * constant definition.
 ******************************************************************************/
#define CARD_CODE_LEN     	512
#define CARD_TITLE_LEN    	64
#define CARD_SUBTITLE_LEN	64
#define CARD_LIST_NUM     	11


/*******************************************************************************
 * type definition.
 ******************************************************************************/
enum ONECARD_TRACK_TYPE {
	ONECARD_UNDEF	= 0,
	ONECARD_TRACK_1	= 1,
	ONECARD_TRACK_2	= 2,
	ONECARD_TRACK_3	= 3,
	ONECARD_JIS_2	= 4,
	ONECARD_UNUSED	= 255
} ;

/*******************************************************************************
 * structure.
 ******************************************************************************/
/**
 * @brief card code.
 */
typedef struct card_code_s {
    uint8_t			value[CARD_CODE_LEN];
    uint16_t		len;
} card_code_t;

/**
 * @brief card title.
 */
typedef struct card_title_s {
    uint8_t			value[CARD_TITLE_LEN];
    uint16_t		len;
} card_title_t;

/**
 * @brief card subtitle.
 */
typedef struct card_subtitle_s {
    uint8_t			value[CARD_SUBTITLE_LEN];
    uint16_t		len;
} card_subtitle_t;

/**
 * @brief card info.
 */
typedef struct card_info_s {
    card_code_t		card_code;
    uint8_t			card_type;
    card_title_t	card_title;
    card_subtitle_t	card_subtitle;
} card_info_t;

/**
 * @brief card config.
 */
typedef struct card_config_s {
    uint8_t         selected_card_no;
    uint8_t			sync_card_no;
    card_info_t		card_list[CARD_LIST_NUM];
} card_config_t;

/*******************************************************************************
 * function prototype.
 ******************************************************************************/
/**
 * @brief initialize card manager module.
 *
 */
void card_manager_init(void);

/**
 * @brief set the selected card number.
 *
 * @param[in]	card_no				card number.
 */
void card_manager_set_selected_card_no(uint8_t card_no);

/**
 * @brief set the sync card number.
 *
 * @param[in]	card_no				card number.
 */
void card_manager_set_sync_card_no(uint8_t card_no);

/**
 * @brief set the card code string.
 *
 * @param[in]	p_value				card code string.
 * @param[in]	len					card code length.
 */
void card_manager_set_card_code(const uint8_t *p_value, uint16_t len);

/**
 * @brief set the card type.
 *
 * @param[in]	type				card type.
 */
void card_manager_set_card_type(uint8_t type);

/**
 * @brief set the card title string.
 *
 * @param[in]	p_value				card title string.
 * @param[in]	len					card title length.
 */
void card_manager_set_card_title(const uint8_t *p_value, uint16_t len);

/**
 * @brief set the card subtitle string.
 *
 * @param[in]	p_value				card subtitle string.
 * @param[in]	len					card subtitle length.
 */
void card_manager_set_card_subtitle(const uint8_t *p_value, uint16_t len);

/**
 * @brief get the selected card number.
 *
 * @retval selected card number.
 */
uint8_t card_manager_selected_card_no(void);

/**
 * @brief get the sync card number.
 *
 * @retval sync card number.
 */
uint8_t card_manager_sync_card_no(void);

/**
 * @brief get the selected card info.
 *
 * @param[in]	sync_card_no			sync card number.
 * @retval sync card info.
 */
const card_info_t* card_manager_selected_card_info(uint8_t sync_card_no);


#endif /* CARD_MANAGER_H_ */

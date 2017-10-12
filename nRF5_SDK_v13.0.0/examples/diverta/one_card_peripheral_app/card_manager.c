/*
 * card_manager.c
 *
 *  Created on: 2016/05/21
 *      Author: r.suzuki
 */

/*******************************************************************************
 * include.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "nrf_log.h"

#include "card_manager.h"

/*******************************************************************************
 * constant definition.
 ******************************************************************************/


/*******************************************************************************
 * structure.
 ******************************************************************************/


/*******************************************************************************
 * static fields.
 ******************************************************************************/
static card_config_t	m_card_config;

/*******************************************************************************
 * function prototype.
 ******************************************************************************/


/*******************************************************************************
 * public function.
 ******************************************************************************/
/**
 * @brief initialize card manager module.
 *
 */
void card_manager_init(void)
{
    memset(&m_card_config, 0, sizeof(card_config_t));
	
	m_card_config.selected_card_no	= 1;
	m_card_config.sync_card_no		= 1;
	for (int i = 0; i < CARD_LIST_NUM; i++) {
		m_card_config.card_list[i].card_type = ONECARD_UNUSED;
	}
}

/**
 * @brief set the selected card number.
 *
 * @param[in]	card_no				card number.
 */
void card_manager_set_selected_card_no(uint8_t card_no)
{
    if (card_no < CARD_LIST_NUM) {
        m_card_config.selected_card_no = card_no;
    }
    else {
        NRF_LOG_INFO("[APP]Invalid card number!!\r\n");
    }
}

/**
 * @brief set the sync card number.
 *
 * @param[in]	card_no				card number.
 */
void card_manager_set_sync_card_no(uint8_t card_no)
{
    if (card_no < CARD_LIST_NUM) {
        m_card_config.sync_card_no = card_no;
    }
    else {
        NRF_LOG_INFO("[APP]Invalid card number!!\r\n");
    }
}

/**
 * @brief set the card code string.
 *
 * @param[in]	p_value				card code string.
 * @param[in]	len					card code length.
 */
void card_manager_set_card_code(const uint8_t *p_value, uint16_t len)
{
    uint8_t		sync			= m_card_config.sync_card_no;
    card_info_t	*p_sync_card	= &m_card_config.card_list[sync];

    if (len < CARD_CODE_LEN) {
        for (int i = 0; i < len; i++) {
            p_sync_card->card_code.value[i] = p_value[i];
        }
        p_sync_card->card_code.len = len;
    }
    else {
        NRF_LOG_INFO("[APP]Invalid card code length!!\r\n");
    }
}

/**
 * @brief set the card type.
 *
 * @param[in]	type				card type.
 */
void card_manager_set_card_type(uint8_t type)
{
    uint8_t		sync			= m_card_config.sync_card_no;
    card_info_t	*p_sync_card	= &m_card_config.card_list[sync];

    p_sync_card->card_type = type;
}

/**
 * @brief set the card title string.
 *
 * @param[in]	p_value				card title string.
 * @param[in]	len					card title length.
 */
void card_manager_set_card_title(const uint8_t *p_value, uint16_t len)
{
    uint8_t		sync			= m_card_config.sync_card_no;
    card_info_t	*p_sync_card	= &m_card_config.card_list[sync];

    if (len < CARD_TITLE_LEN) {
        for (int i = 0; i < len; i++) {
            p_sync_card->card_title.value[i] = p_value[i];
        }
        p_sync_card->card_title.len = len;
    }
    else {
        NRF_LOG_INFO("[APP]Invalid card title length!!\r\n");
    }
}

/**
 * @brief set the card subtitle string.
 *
 * @param[in]	p_value				card subtitle string.
 * @param[in]	len					card subtitle length.
 */
void card_manager_set_card_subtitle(const uint8_t *p_value, uint16_t len)
{
    uint8_t		sync			= m_card_config.sync_card_no;
    card_info_t	*p_sync_card	= &m_card_config.card_list[sync];

    if (len < CARD_SUBTITLE_LEN) {
        for (int i = 0; i < len; i++) {
            p_sync_card->card_subtitle.value[i] = p_value[i];
        }
        p_sync_card->card_subtitle.len = len;
    }
    else {
        NRF_LOG_INFO("[APP]Invalid card subtitle length!!\r\n");
    }
}

/**
 * @brief get the selected card number.
 *
 * @retval selected card number.
 */
uint8_t card_manager_selected_card_no(void)
{
    return m_card_config.selected_card_no;
}

/**
 * @brief get the sync card number.
 *
 * @retval sync card number.
 */
uint8_t card_manager_sync_card_no(void)
{
    return m_card_config.sync_card_no;
}

/**
 * @brief get the selected card info.
 *
 * @param[in]	sync_card_no			sync card number.
 * @retval sync card info.
 */
const card_info_t* card_manager_selected_card_info(uint8_t sync_card_no)
{
    if (sync_card_no < CARD_LIST_NUM) {
        return (const card_info_t*)&m_card_config.card_list[sync_card_no];
    }
    else {
        return NULL;
    }
}


/*******************************************************************************
 * ptivate function.
 ******************************************************************************/





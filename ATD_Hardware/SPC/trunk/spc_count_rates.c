#include "spc.h"
#include "spc_ui.h"

#include "asynctmr.h"

int bh_init_rates_panel(Spc* spc)
{
	if ((spc->_rates_ui_panel = ui_module_add_panel(UIMODULE_CAST(spc), "spc_ui.uir", SPC_RATES, 0)) < 0)
		return SPC_ERROR;   
	
	return SPC_SUCCESS;
}

int bh_read_rates(Spc* spc)
{
	rate_values rates;
	int err=1;
	static int firstCall = 1;
	
	err = SPC_read_rates(spc->_active_module, &rates);
	if (err) return 0;	//not ready

	SetCtrlVal(spc->_rates_ui_panel, SPC_RATES_SYNC, rates.sync_rate);
	if (rates.sync_rate == 0)
		SetCtrlVal(spc->_rates_ui_panel, SPC_RATES_SYNC_SLIDE, 0.0);
	else	
		SetCtrlVal(spc->_rates_ui_panel, SPC_RATES_SYNC_SLIDE, log10(rates.sync_rate));
	
	SetCtrlVal(spc->_rates_ui_panel, SPC_RATES_CFD, rates.cfd_rate);
	if (rates.cfd_rate == 0)
		SetCtrlVal(spc->_rates_ui_panel, SPC_RATES_CFD_SLIDE, 0.0);
	else	
		SetCtrlVal(spc->_rates_ui_panel, SPC_RATES_CFD_SLIDE, log10(rates.cfd_rate));
	
	SetCtrlVal(spc->_rates_ui_panel, SPC_RATES_TAC, rates.tac_rate);
	if (rates.tac_rate == 0)
		SetCtrlVal(spc->_rates_ui_panel, SPC_RATES_TAC_SLIDE, 0.0);
	else	
		SetCtrlVal(spc->_rates_ui_panel, SPC_RATES_TAC_SLIDE, log10(rates.tac_rate));
	
	SetCtrlVal(spc->_rates_ui_panel, SPC_RATES_ADC, rates.adc_rate);
	if (rates.adc_rate == 0)
		SetCtrlVal(spc->_rates_ui_panel, SPC_RATES_ADC_SLIDE, 0.0);
	else	
		SetCtrlVal(spc->_rates_ui_panel, SPC_RATES_ADC_SLIDE, log10(rates.adc_rate));
	
	//The first time the sync rate is read set up the pile-up palette
	firstCall = 0;
	
	return 0;
}


double bh_get_sync_rate(Spc* spc)
{
	double sync_rate;
	
	bh_read_rates(spc);
	
	GetCtrlVal(spc->_rates_ui_panel, SPC_RATES_SYNC, &sync_rate);
		
	return sync_rate;
}


void disable_rate_count_timer(Spc* spc)
{
	SetAsyncTimerAttribute (spc->_timer, ASYNC_ATTR_ENABLED,  0);
}

void enable_rate_count_timer(Spc* spc)
{
	SetAsyncTimerAttribute (spc->_timer, ASYNC_ATTR_ENABLED,  1);
}

int bh_hide_rates_ui (Spc* spc)
{
	disable_rate_count_timer(spc);
	
	//Save panel position and hide BH rates panel
	ui_module_panel_read_or_write_registry_settings(UIMODULE_CAST(spc), spc->_rates_ui_panel, 1);
	HidePanel (spc->_rates_ui_panel);
  	
  	return SPC_SUCCESS;
}

int bh_display_rates_ui (Spc* spc)
{
	enable_rate_count_timer(spc);

  	ui_module_display_panel(UIMODULE_CAST(spc), spc->_rates_ui_panel);   
	
  	return SPC_SUCCESS;
}
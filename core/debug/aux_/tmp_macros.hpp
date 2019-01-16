#if BOOST_PP_LESS_EQUAL( DBG__internal_tmp_MinimumSeverity, DBG_internal_Sev_to_int(VVerb) )
//Crit debugging enabled
#if ( DBG__internal_tmp_MinimumSeverity == DBG_internal_Sev_to_int(VVerb) )
#define DBG__internal_MinimumSeverity DBG_internal_Sev_to_int(VVerb)
#endif

#define DBG__internal_Tmp( Sev, operations )        \
    DBG__internal_PROCESS_DBG( Sev, operations )      /**/

#else
//Crit debugging disabled

#define DBG__internal_Tmp( ... ) do {} while(0)

#endif


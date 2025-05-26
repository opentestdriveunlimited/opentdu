#pragma once

#include "core/hash/crc32.h"

// TODO: This is relying on static init atm (as we're using TDU's CRC32 impl to keep everything consistent)
// We might want to implement a constexpr CRC32 to avoid all this runtime init crap
struct TDUOnlineCRC {
    TDUOnlineCRC( const char* pString )
        : hashcode( TestDriveCRC32( pString ) )
    {

    }

    operator uint32_t () const
    {
        return hashcode;
    }

private:
    uint32_t hashcode;
};

namespace TestDrive
{
    namespace CRC
    {
        static TDUOnlineCRC edNetClientV2( "edNetClient::edNetClientV2()" );
        static TDUOnlineCRC edNetClientV3( "edNetClient::edNetClientV3()" );

        static TDUOnlineCRC edNetSessionClientV3( "edNetSessionClient::edNetSessionClientV3()" );

        static TDUOnlineCRC r_get_ednetbuffer( "edNetServiceNetBuffer::r_get_ednetbuffer(uint bufferid,uint offset)" );
        static TDUOnlineCRC a_get_ednetbuffer( "edNetServiceNetBuffer::a_get_ednetbuffer(uint bufferid,byte errcode,{uint offset,uint totalsize,ushort buffer_crc,buffer}=NULL)" );
        static TDUOnlineCRC e_get_ednetbuffer_ack( "edNetServiceNetBuffer::e_get_ednetbuffer_ack(uint bufferid)" );

        static TDUOnlineCRC edNetServiceHandler( "edNetServiceHandler" );

        static TDUOnlineCRC edNetServiceHandlerAction( "edNetServiceHandlerAction" );
        static TDUOnlineCRC r_callaction( "edNetServiceHandlerAction::r_callaction(uint,uint,object)" );
        static TDUOnlineCRC r_callaction_unreliable( "edNetServiceHandlerAction::r_callaction_unreliable(object)" );
        static TDUOnlineCRC a_callaction( "edNetServiceHandlerAction::a_callaction(uint,object=NULL)" );
        static TDUOnlineCRC a_callaction_with_netbufferid( "edNetServiceHandlerAction::a_callaction_with_netbufferid(uint,uint,object=NULL)" );
        static TDUOnlineCRC e_callaction_ack( "edNetServiceHandlerAction::e_callaction_ack(uint)" );
        static TDUOnlineCRC e_callaction_queued( "edNetServiceHandlerAction::e_callaction_queued(uint,uint)" );

        static TDUOnlineCRC edNetServiceHandlerSession( "edNetServiceHandlerSession" );
        static TDUOnlineCRC a_query_local_session( "edNetServiceHandlerSession::a_query_local_session(object=NULL)" );
        static TDUOnlineCRC r_query_local_session( "edNetServiceHandlerSession::r_query_local_session(string)" );
        
        static TDUOnlineCRC edNetServiceHandlerDataset( "edNetServiceHandlerDataset" );
        static TDUOnlineCRC a_get_dataset( "edNetServiceHandlerDataset::a_get_dataset(object=0)" );
        static TDUOnlineCRC r_get_dataset( "edNetServiceHandlerDataset::r_get_dataset()" );
        static TDUOnlineCRC r_set_dataset( "edNetServiceHandlerDataset::r_set_dataset(uint)" );
        static TDUOnlineCRC r_set_dataset_object( "edNetServiceHandlerDataset::r_set_dataset(uint,object,byte=0)" );

        static TDUOnlineCRC r_ranking_server( "Ranking::r_ranking_server()" );
        static TDUOnlineCRC a_read_stats_solo_chall( "Ranking::a_read_stats_solo_chall(SoloChallResultHeader,SoloChallResultRow [])" );
        static TDUOnlineCRC r_read_stats_solo_chall_around( "Ranking::r_read_stats_solo_chall_around(byte,longlong,int,int)" );
        
        static TDUOnlineCRC e_check_no_answer( "Check::e_check_no_answer(int)" );
        static TDUOnlineCRC r_check_phys( "Check::r_check_phys(int,int,int)" );
        static TDUOnlineCRC e_check_invalid_pack( "Check::e_check_invalid_pack(int)" );
        static TDUOnlineCRC a_check_phys_v3( "Check::a_check_phys_v3(int,int,int,int,longlong,float[,string])" );
        static TDUOnlineCRC a_check_phys( "Check::a_check_phys(int,int,longlong,float[,string])" );

        static TDUOnlineCRC r_getlastnewsbuffer( "News::r_getlastnewsbuffer(uint,uint,uint,int,int_paramLocalized)" );
        static TDUOnlineCRC r_createnews( "News::r_createnews(byte,uint,uint)" );

        static TDUOnlineCRC r_getservertime( "Stats::r_getservertime()" );
        static TDUOnlineCRC a_stats_member_list_xuid( "Stats::a_stats_member_list(xuid)" );
        static TDUOnlineCRC a_stats_member_list( "Stats::a_stats_member_list()" );
        
        
        static TDUOnlineCRC a_getdriverinfos( "Database::a_getdriverinfos(byte,uint,string,uint)" );
        static TDUOnlineCRC r_login( "Database::r_login(string,string,ulonglong)WithEbayStatusAndTeamSpotIdAndRegion" );
        static TDUOnlineCRC r_ebayhistobuyquerycount( "Database::r_ebayhistobuyquerycount(uint,uint)_group" );
        static TDUOnlineCRC r_ebayquery2( "Database::r_ebayquery2(uint,uint,uint,uint)_packhcode_deluxe_odometer_deluxesec2" );
        static TDUOnlineCRC r_login_pc( "Database::r_login_pc(string,string,uint,ulonglong,byte,string)" );
        static TDUOnlineCRC r_exchange( "Database::r_exchange(uint,object,uint,object)" );
        static TDUOnlineCRC a_buyvehicule( "Database::a_buyvehicule(uint,object,uint,object)" );
        static TDUOnlineCRC r_ebayquerycount2( "Database::r_ebayquerycount2(uint,uint)" );
        static TDUOnlineCRC r_ebayhistobuyquery( "Database::r_ebayhistobuyquery(uint,uint)_deluxe" );
        static TDUOnlineCRC r_ebayhistobuyquery( "Database::r_ebayhistobuyquery(uint,uint)_deluxe" );
        static TDUOnlineCRC r_ebayquerycount2_deluxe( "Database::r_ebayquerycount2(uint,uint)_group" );
        static TDUOnlineCRC a_login( "Database::a_login(byte,uint,uint)" );
        static TDUOnlineCRC e_custom_challenge_event( "Database::e_custom_challenge_event(uint,uint,uint,uint,uint,string,uint)" );
        static TDUOnlineCRC e_ebay_car_sold( "Database::e_ebay_car_sold(uint,uint)" );
        static TDUOnlineCRC a_custom_challenge_setcp( "Database::a_custom_challenge_setcp(uint,uint)" );
        static TDUOnlineCRC r_custom_challenge_setcp( "Database::r_custom_challenge_setcp(uint,uint)" );
        static TDUOnlineCRC a_custom_challenge_requestcp( "Database::a_custom_challenge_requestcp()" );
        static TDUOnlineCRC r_custom_challenge_requestcp( "Database::r_custom_challenge_requestcp()" );
        static TDUOnlineCRC a_custom_challenge_check( "Database::a_custom_challenge_check(uint,string,unique)" );
        static TDUOnlineCRC r_custom_challenge_check( "Database::r_custom_challenge_check(uint)" );
        static TDUOnlineCRC a_custom_challenge_list_byte( "Database::a_custom_challenge_list(uint*,name,car,theme,event,popularity,n result,xuid,xuid,byte)" );
        static TDUOnlineCRC a_custom_challenge_list( "Database::a_custom_challenge_list(uint*,name,car,theme,event,popularity,n result,xuid,xuid)" );
        static TDUOnlineCRC r_custom_challenge_list_hc( "Database::r_custom_challenge_list(startat,popularity,xuid,hc)" );
        static TDUOnlineCRC r_custom_challenge_list( "Database::r_custom_challenge_list(startat,popularity,xuid)" );
        static TDUOnlineCRC a_custom_challenge_count( "Database::a_custom_challenge_count()" );
        static TDUOnlineCRC r_custom_challenge_count( "Database::r_custom_challenge_count()" );
        static TDUOnlineCRC a_custom_challenge_is_valid( "Database::a_custom_challenge_is_valid(bool,uint,uint,char)" );
        static TDUOnlineCRC r_custom_challenge_is_valid( "Database::r_custom_challenge_is_valid(uint,uint,char)" );

        static TDUOnlineCRC r_open_file( "ednetresourceorb::r_open_file(byte facility,uint user_id,string filenam e,byte bupdload_wanted,ed32 uploadtotalsize)" );
        static TDUOnlineCRC a_open_file( "ednetresourceorb::a_open_file(byte errcode,uint fileid,uint totalsize,u int maxretry,uint timeout)" );
        static TDUOnlineCRC r_getfile( "ednetresourceorb::r_getfile(uint fileid,uint offset)" );
        static TDUOnlineCRC a_getfile( "ednetresourceorb::a_getfile(byte errcode,uint fileid,uint offset,ushort  nexttimewait,buffer datas=NULL)" );
        static TDUOnlineCRC r_putfile( "ednetresourceorb::r_putfile(uint fileid,uint offset,buffer datas)" );
        static TDUOnlineCRC a_putfile( "ednetresourceorb::a_putfile(byte errcode,uint fileid,uint offset)" );
        static TDUOnlineCRC r_renamefile( "ednetresourceorb::r_renamefile(byte facility,uint user_id,string filena me_currrent_name,string filename_new_name)" );
        static TDUOnlineCRC a_renamefile( "ednetresourceorb::a_renamefile(byte errcode)" );
        static TDUOnlineCRC r_fileexists( "ednetresourceorb::r_fileexists(byte facility,uint user_id,string filename)" );
        static TDUOnlineCRC a_fileexists( "ednetresourceorb::a_fileexists(byte errcode)" );
    }
}

//--------------------------------------------------------------------------
// Copyright (C) 2015-2016 Cisco and/or its affiliates. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License Version 2 as published
// by the Free Software Foundation.  You may not use, modify or distribute
// this program under any other version of the GNU General Public License.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//--------------------------------------------------------------------------

// tcp_state_time_wait.cc author davis mcpherson <davmcphe@cisco.com>
// Created on: Aug 5, 2015

#include <iostream>
using namespace std;

#include "tcp_module.h"
#include "tcp_tracker.h"
#include "tcp_session.h"
#include "tcp_normalizer.h"
#include "tcp_state_time_wait.h"

#ifdef UNIT_TEST
#include "catch/catch.hpp"
#endif

TcpStateTimeWait::TcpStateTimeWait(TcpStateMachine& tsm, TcpSession& ssn) :
    TcpStateHandler(TcpStreamTracker::TCP_TIME_WAIT, tsm, ssn)
{
}

TcpStateTimeWait::~TcpStateTimeWait()
{
}

bool TcpStateTimeWait::syn_sent(TcpSegmentDescriptor& tsd, TcpStreamTracker& tracker)
{
    auto& trk = static_cast< TcpStreamTracker& >( tracker );

    session.check_for_repeated_syn(tsd);

    return default_state_action(tsd, trk);
}

bool TcpStateTimeWait::syn_recv(TcpSegmentDescriptor& tsd, TcpStreamTracker& tracker)
{
    auto& trk = static_cast< TcpStreamTracker& >( tracker );

    trk.normalizer->ecn_tracker(tsd.get_tcph(), session.config->require_3whs() );
    if ( tsd.get_seg_len() )
        session.handle_data_on_syn(tsd);

    return true;
}

bool TcpStateTimeWait::syn_ack_sent(TcpSegmentDescriptor& tsd, TcpStreamTracker& tracker)
{
    auto& trk = static_cast< TcpStreamTracker& >( tracker );

    return default_state_action(tsd, trk);
}

bool TcpStateTimeWait::syn_ack_recv(TcpSegmentDescriptor& tsd, TcpStreamTracker& tracker)
{
    auto& trk = static_cast< TcpStreamTracker& >( tracker );

    return default_state_action(tsd, trk);
}

bool TcpStateTimeWait::ack_sent(TcpSegmentDescriptor& tsd, TcpStreamTracker& tracker)
{
    auto& trk = static_cast< TcpStreamTracker& >( tracker );

    trk.update_tracker_ack_sent(tsd);

    return default_state_action(tsd, trk);
}

bool TcpStateTimeWait::ack_recv(TcpSegmentDescriptor& tsd, TcpStreamTracker& tracker)
{
    auto& trk = static_cast< TcpStreamTracker& >( tracker );

    return default_state_action(tsd, trk);
}

bool TcpStateTimeWait::data_seg_sent(TcpSegmentDescriptor& tsd, TcpStreamTracker& tracker)
{
    auto& trk = static_cast< TcpStreamTracker& >( tracker );
    // data on a segment when we shouldn't be sending data any more alert!
    session.tel.set_tcp_event(EVENT_DATA_ON_CLOSED);
    session.mark_packet_for_drop(tsd);
    return default_state_action(tsd, trk);
}

bool TcpStateTimeWait::data_seg_recv(TcpSegmentDescriptor& tsd, TcpStreamTracker& tracker)
{
    auto& trk = static_cast< TcpStreamTracker& >( tracker );

    return default_state_action(tsd, trk);
}

bool TcpStateTimeWait::fin_sent(TcpSegmentDescriptor& tsd, TcpStreamTracker& tracker)
{
    auto& trk = static_cast< TcpStreamTracker& >( tracker );

    return default_state_action(tsd, trk);
}

bool TcpStateTimeWait::fin_recv(TcpSegmentDescriptor& tsd, TcpStreamTracker& tracker)
{
    auto& trk = static_cast< TcpStreamTracker& >( tracker );

    trk.update_tracker_ack_recv(tsd);
    if ( SEQ_GEQ(tsd.get_seg_seq(), trk.get_fin_final_seq() ) )
    {
        DebugMessage(DEBUG_STREAM_STATE, "FIN beyond previous, ignoring\n");
        session.tel.set_tcp_event(EVENT_BAD_FIN);
        trk.normalizer->packet_dropper(tsd, NORM_TCP_BLOCK);
        session.set_pkt_action_flag(ACTION_BAD_PKT);
    }
    else if ( tsd.get_seg_len() > 0 )
        session.handle_data_segment(tsd);

    return default_state_action(tsd, trk);
}

bool TcpStateTimeWait::rst_sent(TcpSegmentDescriptor& tsd, TcpStreamTracker& tracker)
{
    auto& trk = static_cast< TcpStreamTracker& >( tracker );

    return default_state_action(tsd, trk);
}

bool TcpStateTimeWait::rst_recv(TcpSegmentDescriptor& tsd, TcpStreamTracker& tracker)
{
    auto& trk = static_cast< TcpStreamTracker& >( tracker );

    if ( trk.update_on_rst_recv(tsd) )
    {
        session.update_session_on_rst(tsd, false);
        session.update_perf_base_state(TcpStreamTracker::TCP_CLOSING);
        session.set_pkt_action_flag(ACTION_RST);
    }
    else
    {
        session.tel.set_tcp_event(EVENT_BAD_RST);
    }

    // FIXIT - might be good to create alert specific to RST with data
    if ( tsd.get_seg_len() > 0 )
        session.tel.set_tcp_event(EVENT_DATA_AFTER_RST_RCVD);

    return default_state_action(tsd, trk);
}

bool TcpStateTimeWait::do_pre_sm_packet_actions(TcpSegmentDescriptor& tsd)
{
    return session.validate_packet_established_session(tsd);
}

bool TcpStateTimeWait::do_post_sm_packet_actions(TcpSegmentDescriptor& tsd)
{
    session.update_paws_timestamps(tsd);
    session.check_for_window_slam(tsd);

    if ( tcp_event != TcpStreamTracker::TCP_FIN_RECV_EVENT )
    {
        TcpStreamTracker::TcpState talker_state = session.get_talker_state();
        Flow* flow = tsd.get_flow();

        if ( ( talker_state == TcpStreamTracker::TCP_TIME_WAIT )
            || ( talker_state == TcpStreamTracker::TCP_CLOSED ) )
        {
            // The last ACK is a part of the session. Delete the session after processing is
            // complete.
            session.cleanup_session(false, tsd.get_pkt() );
            flow->session_state |= STREAM_STATE_CLOSED;
            session.set_pkt_action_flag(ACTION_LWSSN_CLOSED);
        }
    }
    return true;
}


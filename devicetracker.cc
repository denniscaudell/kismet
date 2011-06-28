/*
    This file is part of Kismet

    Kismet is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kismet is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Kismet; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "config.h"

#include <stdio.h>
#include <time.h>
#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>

#include "globalregistry.h"
#include "util.h"
#include "configfile.h"
#include "messagebus.h"
#include "packetchain.h"
#include "devicetracker.h"
#include "packet.h"
#include "gpsdclient.h"
#include "alertracker.h"
#include "manuf.h"
#include "packetsource.h"

enum KISDEV_COMMON_FIELDS {
	KISDEV_phytype, KISDEV_macaddr, KISDEV_firsttime, KISDEV_lasttime,
	KISDEV_packets, KISDEV_llcpackets, KISDEV_errorpackets,
	KISDEV_datapackets, KISDEV_cryptpackets,
	KISDEV_datasize, KISDEV_newpackets, KISDEV_channel, KISDEV_frequency,
	KISDEV_freqmhz,
	
	KISDEV_gpsfixed,
    KISDEV_minlat, KISDEV_minlon, KISDEV_minalt, KISDEV_minspd,
    KISDEV_maxlat, KISDEV_maxlon, KISDEV_maxalt, KISDEV_maxspd,
    KISDEV_signaldbm, KISDEV_noisedbm, 
	KISDEV_minsignaldbm, KISDEV_minnoisedbm, KISDEV_maxsignaldbm, KISDEV_maxnoisedbm,
    KISDEV_signalrssi, KISDEV_noiserssi, KISDEV_minsignalrssi, KISDEV_minnoiserssi,
    KISDEV_maxsignalrssi, KISDEV_maxnoiserssi,
    KISDEV_bestlat, KISDEV_bestlon, KISDEV_bestalt,
    KISDEV_agglat, KISDEV_agglon, KISDEV_aggalt, KISDEV_aggpoints,

	KISDEV_maxfield
};

const char *KISDEV_common_text[] = {
	"phytype", "macaddr", "firsttime", "lasttime",
	"packets", "llcpackets", "errorpackets",
	"datapackets", "cryptpackets",
	"datasize", "newpackets", "channel", "frequency",
	"freqmhz",

	"gpsfixed",
	"minlat", "minlon", "minalt", "minspd",
	"maxlat", "maxlon", "maxalt", "maxspd",
	"signaldbm", "noisedbm", "minsignaldbm", "minnoisedbm",
	"signalrssi", "noiserssi", "minsignalrssi", "minnoiserssi",
	"maxsignalrssi", "maxnoiserssi",
	"bestlat", "bestlon", "bestalt",
	"agglat", "agglon", "aggalt", "aggpoints",

	NULL
};

enum DEVTAG_FIELDS {
	DEVTAG_macaddr, DEVTAG_tag, DEVTAG_value,

	DEVTAG_maxfield
};

const char *DEVTAG_fields_text[] = {
	"macaddr", "tag", "value",
	NULL
};

// Replaces the *INFO sentence
enum TRACKINFO_FIELDS {
	TRACKINFO_devices, TRACKINFO_packets, TRACKINFO_datapackets,
	TRACKINFO_cryptpackets, TRACKINFO_errorpackets, TRACKINFO_filterpackets,
	TRACKINFO_packetrate,

	TRACKINFO_maxfield
};

const char *TRACKINFO_fields_text[] = {
	"devices", "packets", "datapackets",
	"cryptpackets", "errorpackets", "filterpackets",
	"packetrate",
	NULL
};

enum STRING_FIELDS {
	STRING_device, STRING_phy, STRING_source, STRING_dest, STRING_string,
	STRING_maxfield
};

const char *STRINGS_fields_text[] = {
    "device", "phy", "source", "dest", "string", 
    NULL
};

int Protocol_KISDEV_COMMON(PROTO_PARMS) {
	kis_device_common *com = (kis_device_common *) data;
	kis_tracked_device *dev = com->device;
	string scratch;

	cache->Filled(field_vec->size());

	for (unsigned int x = 0; x < field_vec->size(); x++) {
		unsigned int fnum = (*field_vec)[x];

		if (fnum > KISDEV_maxfield) {
			out_string = "\001Unknown field\001";
			return -1;
		}

		if (cache->Filled(fnum)) {
			out_string += cache->GetCache(fnum) + " ";
			continue;
		}

		scratch = "";

		switch (fnum) {
			case KISDEV_phytype:
				scratch = IntToString(com->phy_type);
				break;
			case KISDEV_macaddr:
				scratch = dev->key.Mac2String();
				break;
			case KISDEV_firsttime:
				scratch = IntToString(com->first_time);
				break;
			case KISDEV_lasttime:
				scratch = IntToString(com->last_time);
				break;
			case KISDEV_packets:
				scratch = IntToString(com->packets);
				break;
			case KISDEV_llcpackets:
				scratch = IntToString(com->llc_packets);
				break;
			case KISDEV_errorpackets:
				scratch = IntToString(com->error_packets);
				break;
			case KISDEV_datapackets:
				scratch = IntToString(com->data_packets);
				break;
			case KISDEV_cryptpackets:
				scratch = IntToString(com->crypt_packets);
				break;
			case KISDEV_datasize:
				scratch = LongIntToString(com->datasize);
				break;
			case KISDEV_newpackets:
				scratch = IntToString(com->new_packets);
				break;
			case KISDEV_channel:
				scratch = IntToString(com->channel);
				break;
			case KISDEV_frequency:
				scratch = IntToString(com->frequency);
				break;
			case KISDEV_freqmhz:
				for (map<unsigned int, unsigned int>::const_iterator fmi = com->freq_mhz_map.begin();
					 fmi != com->freq_mhz_map.end(); ++fmi) {
					scratch += IntToString(fmi->first) + ":" + IntToString(fmi->second) + "*";
				}
				break;
			case KISDEV_gpsfixed:
				scratch = IntToString(com->gpsdata.gps_valid);
				break;
			case KISDEV_minlat:
				scratch = FloatToString(com->gpsdata.min_lat);
				break;
			case KISDEV_minlon:
				scratch = FloatToString(com->gpsdata.min_lon);
				break;
			case KISDEV_minalt:
				scratch = FloatToString(com->gpsdata.min_alt);
				break;
			case KISDEV_minspd:
				scratch = FloatToString(com->gpsdata.min_spd);
				break;
			case KISDEV_maxlat:
				scratch = FloatToString(com->gpsdata.max_lat);
				break;
			case KISDEV_maxlon:
				scratch = FloatToString(com->gpsdata.max_lon);
				break;
			case KISDEV_maxalt:
				scratch = FloatToString(com->gpsdata.max_alt);
				break;
			case KISDEV_maxspd:
				scratch = FloatToString(com->gpsdata.max_spd);
				break;
			case KISDEV_signaldbm:
				scratch = IntToString(com->snrdata.last_signal_dbm);
				break;
			case KISDEV_noisedbm:
				scratch = IntToString(com->snrdata.last_noise_dbm);
				break;
			case KISDEV_minsignaldbm:
				scratch = IntToString(com->snrdata.min_signal_dbm);
				break;
			case KISDEV_maxsignaldbm:
				scratch = IntToString(com->snrdata.max_signal_dbm);
				break;
			case KISDEV_minnoisedbm:
				scratch = IntToString(com->snrdata.min_noise_dbm);
				break;
			case KISDEV_maxnoisedbm:
				scratch = IntToString(com->snrdata.max_noise_dbm);
				break;
			case KISDEV_signalrssi:
				scratch = IntToString(com->snrdata.last_signal_rssi);
				break;
			case KISDEV_noiserssi:
				scratch = IntToString(com->snrdata.last_noise_rssi);
				break;
			case KISDEV_minsignalrssi:
				scratch = IntToString(com->snrdata.min_signal_rssi);
				break;
			case KISDEV_maxsignalrssi:
				scratch = IntToString(com->snrdata.max_signal_rssi);
				break;
			case KISDEV_minnoiserssi:
				scratch = IntToString(com->snrdata.min_noise_rssi);
				break;
			case KISDEV_maxnoiserssi:
				scratch = IntToString(com->snrdata.max_noise_rssi);
				break;
			case KISDEV_bestlat:
				scratch = IntToString(com->snrdata.peak_lat);
				break;
			case KISDEV_bestlon:
				scratch = IntToString(com->snrdata.peak_lon);
				break;
			case KISDEV_bestalt:
				scratch = IntToString(com->snrdata.peak_alt);
				break;
			case KISDEV_agglat:
				scratch = LongIntToString(com->gpsdata.aggregate_lat);
				break;
			case KISDEV_agglon:
				scratch = LongIntToString(com->gpsdata.aggregate_lon);
				break;
			case KISDEV_aggalt:
				scratch = LongIntToString(com->gpsdata.aggregate_alt);
				break;
			case KISDEV_aggpoints:
				scratch = LongIntToString(com->gpsdata.aggregate_points);
				break;
		}
		
		cache->Cache(fnum, scratch);
		out_string += scratch + " ";
	}

	return 1;
}

void Protocol_KISDEV_COMMON_enable(PROTO_ENABLE_PARMS) {
	((Devicetracker *) data)->BlitDevices(in_fd);
}

int Protocol_KISDEV_TRACKINFO(PROTO_PARMS) {
	Devicetracker *tracker = (Devicetracker *) data;
	string scratch;

	cache->Filled(field_vec->size());

	for (unsigned int x = 0; x < field_vec->size(); x++) {
		unsigned int fnum = (*field_vec)[x];

		if (fnum > TRACKINFO_maxfield) {
			out_string = "\001Unknown field\001";
			return -1;
		}

		if (cache->Filled(fnum)) {
			out_string += cache->GetCache(fnum) + " ";
			continue;
		}

		scratch = "";

		switch (fnum) {
			case TRACKINFO_devices:
				scratch = IntToString(tracker->FetchNumDevices(KIS_PHY_ANY));
				break;
			case TRACKINFO_packets:
				scratch = IntToString(tracker->FetchNumPackets(KIS_PHY_ANY));
				break;
			case TRACKINFO_datapackets:
				scratch = IntToString(tracker->FetchNumDatapackets(KIS_PHY_ANY));
				break;
			case TRACKINFO_cryptpackets:
				scratch = IntToString(tracker->FetchNumCryptpackets(KIS_PHY_ANY));
				break;
			case TRACKINFO_errorpackets:
				scratch = IntToString(tracker->FetchNumErrorpackets(KIS_PHY_ANY));
				break;
			case TRACKINFO_filterpackets:
				scratch = IntToString(tracker->FetchNumFilterpackets(KIS_PHY_ANY));
				break;
			case TRACKINFO_packetrate:
				scratch = IntToString(tracker->FetchPacketRate(KIS_PHY_ANY));
				break;
		}
		
		cache->Cache(fnum, scratch);
		out_string += scratch + " ";
	}

	return 1;
}

// String info shoved into the protocol handler by the string collector
class kis_proto_string_info {
public:
	mac_addr device;
	int phy;
	mac_addr source;
	mac_addr dest;
	string stringdata;
};

int Protocol_KISDEV_STRING(PROTO_PARMS) {
	kis_proto_string_info *info = (kis_proto_string_info *) data;

	string scratch;

	cache->Filled(field_vec->size());

	for (unsigned int x = 0; x < field_vec->size(); x++) {
		unsigned int fnum = (*field_vec)[x];

		if (fnum > STRING_maxfield) {
			out_string = "\001Unknown field\001";
			return -1;
		} 

		switch (fnum) {
			case STRING_device:
				scratch = info->device.Mac2String();
				break;
			case STRING_phy:
				scratch = IntToString(info->phy);
				break;
			case STRING_source:
				scratch = info->source.Mac2String();
				break;
			case STRING_dest:
				scratch = info->dest.Mac2String();
				break;
			case STRING_string:
				scratch = "\001" + info->stringdata + "\001";
				break;
			default:
				scratch = "\001Unknown string field\001";
				break;
		}

		cache->Cache(fnum, scratch);
		out_string += scratch + " ";
	}

	return 1;
}

int Devicetracker_Timer(TIMEEVENT_PARMS) {
	return ((Devicetracker *) parm)->TimerKick();
}

int Devicetracker_packethook_stringcollector(CHAINCALL_PARMS) {
	return ((Devicetracker *) auxdata)->StringCollector(in_pack);
}

int Devicetracker_packethook_commonclassifier(CHAINCALL_PARMS) {
	return ((Devicetracker *) auxdata)->CommonClassifier(in_pack);
}

Devicetracker::Devicetracker(GlobalRegistry *in_globalreg) {
	globalreg = in_globalreg;

	globalreg->InsertGlobal("DEVICE_TRACKER", this);

	next_componentid = 0;
	num_packets = num_errorpackets = num_filterpackets = num_packetdelta = 0;

	conf_save = 0;
	next_phy_id = 0;

	// Internally register our common reference first
	devcomp_ref_common = RegisterDeviceComponent("COMMON");

	// Timer kickoff
	timerid = 
		globalreg->timetracker->RegisterTimer(SERVER_TIMESLICES_SEC, NULL, 1,
											  &Devicetracker_Timer, this);

	// Register the tracked device component of packets
	if (_PCM(PACK_COMP_DEVICE) != -1) {
		pack_comp_device = _PCM(PACK_COMP_DEVICE);
	} else {
		pack_comp_device = _PCM(PACK_COMP_DEVICE) =
			globalreg->packetchain->RegisterPacketComponent("DEVICE");
	}

	// Register global packet components used by the device tracker and
	// subsequent parts
	pack_comp_common =  _PCM(PACK_COMP_COMMON) =
		globalreg->packetchain->RegisterPacketComponent("COMMON");

	pack_comp_basicdata = _PCM(PACK_COMP_BASICDATA) = 
		globalreg->packetchain->RegisterPacketComponent("BASICDATA");

	_PCM(PACK_COMP_MANGLEFRAME) =
		globalreg->packetchain->RegisterPacketComponent("MANGLEDATA");

	pack_comp_radiodata = 
		globalreg->packetchain->RegisterPacketComponent("RADIODATA");

	// Register the network protocols
	proto_ref_commondevice =
		globalreg->kisnetserver->RegisterProtocol("COMMON", 0, 1,
												  KISDEV_common_text,
												  &Protocol_KISDEV_COMMON,
												  &Protocol_KISDEV_COMMON_enable,
												  this);

	proto_ref_trackinfo =
		globalreg->kisnetserver->RegisterProtocol("TRACKINFO", 0, 1,
												  TRACKINFO_fields_text,
												  &Protocol_KISDEV_TRACKINFO,
												  NULL,
												  this);

	// Common classifier, late in the classifier chain
	globalreg->packetchain->RegisterHandler(&Devicetracker_packethook_commonclassifier,
											this, CHAINPOS_CLASSIFIER, 100);

	// Strings
	globalreg->packetchain->RegisterHandler(&Devicetracker_packethook_stringcollector,
											this, CHAINPOS_LOGGING, -100);

	if (_PCM(PACK_COMP_STRINGS) != -1) {
		pack_comp_string = _PCM(PACK_COMP_STRINGS);
	} else {
		pack_comp_string = _PCM(PACK_COMP_STRINGS) =
			globalreg->packetchain->RegisterPacketComponent("string");
	}

	if (_NPM(PROTO_REF_STRING) == -1) {
		proto_ref_string = _NPM(PROTO_REF_STRING);
	} else {
		proto_ref_string = _NPM(PROTO_REF_STRING) = 
			globalreg->kisnetserver->RegisterProtocol("STRING", 0, 0,
													  STRINGS_fields_text,
													  &Protocol_KISDEV_STRING,
													  NULL, this);
	}

}

Devicetracker::~Devicetracker() {
	if (timerid >= 0)
		globalreg->timetracker->RemoveTimer(timerid);

	globalreg->packetchain->RemoveHandler(&Devicetracker_packethook_stringcollector,
										  CHAINPOS_LOGGING);
	globalreg->packetchain->RemoveHandler(&Devicetracker_packethook_commonclassifier,
										  CHAINPOS_CLASSIFIER);

}

void Devicetracker::SaveTags() {
	int ret;

	string dir = 
		tag_conf->ExpandLogPath(globalreg->kismet_config->FetchOpt("configdir"),
								"", "", 0, 1);

	ret = mkdir(dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);

	if (ret < 0 && errno != EEXIST) {
		string err = string(strerror(errno));
		_MSG("Failed to create Kismet settings directory " + dir + ": " + err,
			 MSGFLAG_ERROR);
	}

	ret = tag_conf->SaveConfig(tag_conf->ExpandLogPath(globalreg->kismet_config->FetchOpt("configdir") + "/" + "tag.conf", "", "", 0, 1).c_str());

	if (ret < 0)
		_MSG("Could not save tags, check previous error messages (probably "
			 "no permission to write to the Kismet config directory: " + dir,
			 MSGFLAG_ERROR);
}

int Devicetracker::FetchNumDevices(int in_phy) {
	int r = 0;

	if (in_phy == KIS_PHY_ANY)
		return tracked_map.size();

	for (unsigned int x = 0; x < tracked_vec.size(); x++) {
		if (tracked_vec[x]->phy_type == in_phy)
			r++;
	}

	return r;
}

int Devicetracker::FetchNumPackets(int in_phy) {
	if (in_phy == KIS_PHY_ANY)
		return num_packets;

	map<int, int>::iterator i = phy_packets.find(in_phy);
	if (i != phy_packets.end())
		return i->second;

	return 0;
}

int Devicetracker::FetchNumDatapackets(int in_phy) {
	int r = 0;

	kis_device_common *common;

	for (unsigned int x = 0; x < tracked_vec.size(); x++) {
		if (tracked_vec[x]->phy_type == in_phy || in_phy == KIS_PHY_ANY) {
			if ((common = 
				 (kis_device_common *) tracked_vec[x]->fetch(devcomp_ref_common)) != NULL)
				r += common->data_packets;
		}
	}

	return 0;
}

int Devicetracker::FetchNumCryptpackets(int in_phy) {
	int r = 0;

	kis_device_common *common;

	for (unsigned int x = 0; x < tracked_vec.size(); x++) {
		if (tracked_vec[x]->phy_type == in_phy || in_phy == KIS_PHY_ANY) {
			if ((common = 
				 (kis_device_common *) tracked_vec[x]->fetch(devcomp_ref_common)) != NULL)
				r += common->crypt_packets;
		}
	}

	return 0;
}

int Devicetracker::FetchNumErrorpackets(int in_phy) {
	if (in_phy == KIS_PHY_ANY)
		return num_errorpackets;

	map<int, int>::iterator i = phy_errorpackets.find(in_phy);
	if (i != phy_errorpackets.end())
		return i->second;

	return 0;
}

int Devicetracker::FetchNumFilterpackets(int in_phy) {
	if (in_phy == KIS_PHY_ANY)
		return num_filterpackets;

	map<int, int>::iterator i = phy_filterpackets.find(in_phy);
	if (i != phy_errorpackets.end())
		return i->second;

	return 0;
}

int Devicetracker::FetchPacketRate(int in_phy) {
	if (in_phy == KIS_PHY_ANY)
		return num_packetdelta;

	map<int, int>::iterator i = phy_packetdelta.find(in_phy);
	if (i != phy_packetdelta.end())
		return i->second;

	return 0;
}

int Devicetracker::RegisterDeviceComponent(string in_component) {
	if (component_str_map.find(StrLower(in_component)) != component_str_map.end()) {
		return component_str_map[StrLower(in_component)];
	}

	int num = next_componentid++;

	component_str_map[StrLower(in_component)] = num;
	component_id_map[num] = StrLower(in_component);

	return num;
}

int Devicetracker::RegisterPhyHandler(Kis_Phy_Handler *in_weak_handler) {
	int num = next_phy_id++;

	Kis_Phy_Handler *strongphy = 
		in_weak_handler->CreatePhyHandler(globalreg, this, num);

	phy_handler_map[num] = strongphy;

	return num;
}

// Send all devices to a client
void Devicetracker::BlitDevices(int in_fd) {
	for (unsigned int x = 0; x < tracked_vec.size(); x++) {
		kis_protocol_cache cache;

		// If it has a common field
		kis_device_common *common;

		if ((common = 
			 (kis_device_common *) tracked_vec[x]->fetch(devcomp_ref_common)) != NULL) {

			if (in_fd == -1)
				globalreg->kisnetserver->SendToAll(proto_ref_commondevice, (void *) &common);
			else
				globalreg->kisnetserver->SendToClient(in_fd, proto_ref_commondevice,
													  (void *) &common, &cache);
		} 
	}
}

int Devicetracker::TimerKick() {
	for (unsigned int x = 0; x < dirty_device_vec.size(); x++) {
		kis_tracked_device *dev = dirty_device_vec[x];

		// If it has a common field
		kis_device_common *common;

		if ((common = 
			 (kis_device_common *) dev->fetch(devcomp_ref_common)) != NULL) {
			globalreg->kisnetserver->SendToAll(proto_ref_commondevice, 
											   (void *) &common);
		}
	}

	for (map<int, Kis_Phy_Handler *>::iterator x = phy_handler_map.begin(); 
		 x != phy_handler_map.end(); ++x) {
		x->second->TimerKick();
	}

	return 1;
}

kis_tracked_device *Devicetracker::FetchDevice(mac_addr in_device) {
	device_itr i = tracked_map.find(in_device);

	if (i != tracked_map.end())
		return i->second;

	return NULL;
}

int Devicetracker::StringCollector(kis_packet *in_pack) {
	kis_tracked_device_info *devinfo = 
		(kis_tracked_device_info *) in_pack->fetch(_PCM(PACK_COMP_DEVICE));
	kis_common_info *common = 
		(kis_common_info *) in_pack->fetch(pack_comp_common);
	kis_string_info *strings =
		(kis_string_info *) in_pack->fetch(_PCM(PACK_COMP_STRINGS));

	if (devinfo == NULL || strings == NULL || common == NULL)
		return 0;

	kis_proto_string_info si;

	si.device = devinfo->devref->key;
	si.phy = devinfo->devref->phy_type;
	si.source = common->source;
	si.dest = common->dest;

	for (unsigned int x = 0; x < strings->extracted_strings.size(); x++) {
		si.stringdata = strings->extracted_strings[x];
	
		globalreg->kisnetserver->SendToAll(_NPM(PROTO_REF_STRING), (void *) &si);
	}

	return 1;
}

int Devicetracker::CommonClassifier(kis_packet *in_pack) {
	return 0;
}

kis_tracked_device *Devicetracker::MapToDevice(kis_packet *in_pack) {
	// Fetch the bits of the packet we tie into a device record
	kis_common_info *pack_common = 
		(kis_common_info *) in_pack->fetch(pack_comp_common);
	kis_data_packinfo *pack_data = 
		(kis_data_packinfo *) in_pack->fetch(pack_comp_basicdata);
	kis_layer1_packinfo *pack_l1info = 
		(kis_layer1_packinfo *) in_pack->fetch(pack_comp_radiodata);

	kis_tracked_device *device = NULL;
	kis_device_common *common = NULL;

	if (pack_common == NULL)
		return NULL;

	device_itr di = tracked_map.find(pack_common->device);

	if (di == tracked_map.end()) {
		device = new kis_tracked_device;

		device->key = pack_common->device;

		tracked_map[device->key] = device;
		tracked_vec.push_back(device);

		common = new kis_device_common;
	} else {
		device = di->second;
	}

	if (device->dirty == 0) {
		device->dirty = 1;
		dirty_device_vec.push_back(device);
	}

}

// [_613.js] 业务动作层: 输出切换(_8153/状态灯)、电机安装 MountMotor(_8228)、
//   分块上传引擎 _8248(4096B 块 x 455B 片, 固件/歌曲/插件共用)、
//   拾音器插件格式解析 _8286(魔数 0x5D9F2A6B, 头整数字段小端, 重定位表, 0xCC 填充对齐 64KB)、
//   固件版本获取 _8272(fetch _710.bin 解析 "Dashichang silk leg guita(version: ")
async function _8219()
{
    let _1360 = F('speaker');
    _8153(_1360);
    await _1170._8172(_1360);
}
async function _8220()
{
    let _1360 = F('line_out');
    _8153(_1360);
    await _1170._8172(_1360);
}
async function _8221()
{
    let _1360 = F('bluetooth_drive_speaker');
    _8153(_1360);
    await _1170._8172(_1360);
    await _1170._8168();
}
async function _8222()
{
    let _1360 = F('wireless_microphone');
    _8153(_1360);
    await _1170._8172(_1360);
}
async function _8223()
{
    let _1360 = F('bluetooth_midi_keyboard');
    _8153(_1360);
    await _1170._8172(_1360);
}

function _8153(_1360)
{
    function _8224(_1990, _8225, select)
    {
        if (select)
        {
            if (_1990.onclick)
            {
                _1990._8226 = _1990.onclick;
                _1990.onclick = null;
            }
            _1990.style.cursor = "";
            _8225.src = P("_272.png");
        }
        else
        {
            if (_1990._8226)
            {
                _1990.onclick = _1990._8226;
                _1990._8226 = null;
            }
            let _1157;
            _1157 = _1990.getAttribute("enable");
            if ((!_1157) || _1157=="true")
                _1990.style.cursor = "pointer";
            else
                _1990.style.cursor = "";
            _8225.src = P("_271.png");
        }
        if (_8225 == _658)
        {
            if (select)
                _659.style.display = "";
            else
                _659.style.display = "none";
        }
    }
    switch (_1360)
    {
        default:
            _8224(_653, _654, true);
            _8224(_655, _656, false);
            _8224(_657, _658, false);
            _8224(_670, _671, false);
            _8224(_672, _673, false);
            break;
        case F("line_out"):
            _8224(_653, _654, false);
            _8224(_655, _656, true);
            _8224(_657, _658, false);
            _8224(_670, _671, false);
            _8224(_672, _673, false);
            break;
        case F("bluetooth_drive_speaker"):
            _8224(_653, _654, false);
            _8224(_655, _656, false);
            _8224(_657, _658, true);
            _8224(_670, _671, false);
            _8224(_672, _673, false);
            break;
        case F("wireless_microphone"):
            _8224(_653, _654, false);
            _8224(_655, _656, false);
            _8224(_657, _658, false);
            _8224(_670, _671, true);
            _8224(_672, _673, false);
            break;
        case F("bluetooth_midi_keyboard"):
            _8224(_653, _654, false);
            _8224(_655, _656, false);
            _8224(_657, _658, false);
            _8224(_670, _671, false);
            _8224(_672, _673, true);
            break;
    }
}



function _8227()
{
    _1170._8161();
}
function _8162()
{
    _1170._8162();
}
function _8163()
{
    _1170._8163();
}

async function _8228(_8188, _8189)
{
    _675.disabled = true;
    _676.disabled = true;
    _677.disabled = true;
    _678.disabled = true;
    _679.disabled = true;

    let _8229;
    _8229 = parseInt(_679.value);
    if (_8229 > 10 * 1000)
        _8229 = 10 * 1000;
    let _8230;
    _8230 = Date.now();
    _8230 += _8229 + 200;

    await _1170._8187(_8188, _8189, _8229);

    let _8231;
    _8231 = _8230 - Date.now();
    if (_8231 > 0)
        await new Promise(_2303 => setTimeout(_2303, _8231));

    _675.disabled = false;
    _676.disabled = false;
    _677.disabled = false;
    _678.disabled = false;
    _679.disabled = false;
}
async function _8232()
{
    await _8228("1","Up");
}
async function _8233()
{
    await _8228("1", "Down");
}
async function _8234()
{
    await _8228("2", "Up");
}
async function _8235()
{
    await _8228("2", "Down");
}




var _8196 = null;
function _8236(_1111)
{
    _8197._8237();
    _8196 = _1111;
    _1170._8164(_1111);
    _690.style.display = "inline-block";
}
function _8238(_1111)
{
    _8197._8237();
    _8196 = _1111;
    _1170._8165(_1111);
    _690.style.display = "inline-block";
}



function _8206(_8205 = undefined)
{
    if (_8205 === undefined)
        _8205 = _8206._8239;
    else
        _8206._8239 = _8205;
    function _8240(_8225, _8241)
    {
        if (_8241)
        {
            _8225.src = "_272.png";
            _8225.style.cursor = "";
            if (onclick)
            {
                _8225._8226 = _8225.onclick;
                _8225.onclick = null;
            }
        }
        else
        {
            _8225.src = "_271.png";
            _8225.style.cursor = "pointer";
            if (_8225._8226)
            {
                _8225.onclick = _8225._8226;
                _8225._8226 = null;
            }
        }
    }
    function _8242(_8243)
    {
        _878._879(_653, !_8243);
        _878._879(_655, !_8243);
        _878._879(_657, !_8243);
        _659.style.display = _8243 ? "none" : "";
        _878._879(_670, !_8243);
        _878._879(_672, _8243);
    }
    switch (_8205)
    {
        default:
            _8240(_629, true);
            _8240(_630,false);
            _8240(_631, false);
            _8242(false);
            _616._1346();
            _616.onclick();
            _617._8244();
            _618._8244();
            break;
        case "pickup":
            _8240(_629, false);
            _8240(_630, true);
            _8240(_631, false);
            _8242(false);
            _616._8244();
            _617._1346();
            _617.onclick();
            _618._8244();
            break;
        case "midi_device":
            _8240(_629, false);
            _8240(_630, false);
            _8240(_631, true);
            _8242(true);
            _616._8244();
            _617._8244();
            _618._1346();
            _618.onclick();
            break;
    }

}
function _8245()
{
    _1170._8173();
    _8206("auto_pitch");
}
function _8246()
{
    _1170._8174();
    _8206("pickup");
}
function _8247()
{
    _1170._8175();
    _8206("midi_device");
}
async function _8248(_8249, _8250, _8251, _8252, _8253, _8254, _8255)
{
    
    

    const _8256 = _8249._1116();
    if (_8250 != 0)
    {
        if (_8256 > _8250)
        {
            alert(_8251);
            return;
        }
    }

    

    const _8257 = _1170._8136;
    console.assert(_8257 + 50 <= _1170._8135);
    const _8258 = Math.ceil(_1210._8259 / _8257);
    const _8260 = Math.ceil(_8256 / _1210._8259);

    try
    {
        for (let _8261 = 0; _8261 < _8260; _8261++)
        {
            if (_8252.onclick == null)
                break;
            _8253._1160(_8261, _8260);
            let _8262;
            _8262 = _8261 * _1210._8259;
            let _8263;
            _8263 = _8262 + _1210._8259 - 1;
            if (_8263 >= _8256)
                _8263 = _8256 - 1;
            let _8264;
            _8264 = Math.ceil((_8256 - _8262) / _8257);
            if (_8264 > _8258)
                _8264 = _8258;
            for (let _8265 = 0; _8265 < _8264; _8265++)
            {
                if (_8252.onclick == null)
                    break;
                let _8266;
                _8266 = _8262 + _8265 * _8257;
                let _8267;
                _8267 = _8266 + _8257 - 1;
                if (_8267 > _8263)
                    _8267 = _8263;
                let _8268 = _8267 - _8266 + 1;
                _8249.position = _8266;
                let _8269 = new _933();
                _8249._1106(_8269, _8268);
                let _8270 = new _933();
                for (let _934 = 0; _934 < 2; _934++)
                {
                    if (_934 == 0)
                        _8270._935();
                    else
                        _8270._936();
                    _8270._1082(_1170._8137);
                    _8270._1088(_8254);
                    _8270._1085(_8261);
                    _8270._1085(_8260);
                    _8270._1085(_8265);
                    _8270._1085(_8257);
                    _8270._1085(_8264);
                    _8269.position = 0;
                    _8270._1089(_8269);
                }
                await _1170._8157(_8270);
            }
        }
    }
    catch (_983)
    {
        alert(_8255);
    }
}

async function _714()
{
    _636.style.display = "inline-block";
    _637.style.display = "inline-block";
    _878._879(_637, true);
    _8197._8237(_637);

    
    
    await _8248(_8271, 0, null, _637, _636, "AutoInstrumentAudioData", L("自动乐器数据，下载失败"));

    _636.style.display = "none";
    _637.style.display = "none";
    _8197._8198();

    if (_637.onclick != null)
        _878._879(_635, false);
}


async function _8272()
{
    try
    {
        function _8273(_1881)
        {
            let _8016 = new Uint8Array(_1881.length);
            for (let _1119 = 0; _1119 < _1881.length; _1119++)
            {
                _8016[_1119] = _1881.charCodeAt(_1119);
            }
            return _8016;
        }

        function _8274(_8275, _8276, start = 0)
        {
            _858:
            for (let _1119 = start; _1119 <= _8275.length - _8276.length; _1119++)
            {
                for (let _1362 = 0; _1362 < _8276.length; _1362++)
                {
                    if (_8275[_1119 + _1362] !== _8276[_1362])
                        continue _858;
                }
                return _1119;
            }
            return -1;
        }
        let _2010;
        _2010 = await fetch(P("_710.bin"));
        let _8277;
        _8277 = await _2010.arrayBuffer();
        let _1880 = new Uint8Array(_8277);
        let _8278 = _8273("Dashichang silk leg guita(version: ");
        let _994 = _8273(") starting...");
        let start = _8274(_1880, _8278);
        if (start === -1)
            throw new Error();
        start += _8278.length;
        let _1790 = _8274(_1880, _994, start);
        if (_1790 === -1)
            throw new Error();
        let _8279 = _1880.slice(start, _1790);
        _8280 = new TextDecoder().decode(_8279);
        _681.innerText = _8280;
        _8208._1094(_8277);
    }
    catch (_983)
    {
        _8208._1025();
        _681.innerText = L("（无法获取最新固件）");
        _878._879(_682, false); 
    }
}

async function _8281()
{
    _684.style.display = "inline-block";
    _683.style.display = "inline-block";
    _878._879(_684, true);
    _8197._8237(_684);

    const _8282 = 2 * 1024 * 1024;
    await _8248(_8208, _8282, L("要更新的固件太大，放不下"), _684, _683, "UpdateFirmwareData", L("更新的固件，下载失败"));


    _684.style.display = "none";
    _683.style.display = "none";
    _8197._8198();

    if (_684.onclick == null)
        await _1170._8191();
    else
    {
        _878._879(_682, false);
        _614.onclick();
    }
}


async function _8283()
{
    _644.style.display = "inline-block";
    _643.style.display = "inline-block";
    _878._879(_644, true);
    _8197._8237(_644);

    const _8284 = 1024 * 800;
    await _8248(_8285, _8284, L("要更新的拾音器插件太大，放不下"), _644, _643, "DownloadPickupPlugin", L("更新拾音器插件失败"));

    _644.style.display = "none";
    _643.style.display = "none";
    _8197._8198();

    if (_644.onclick != null)
        _878._879(_642, false);
}

function _8286(_8285, _8287, _8288, _8289)
{
    let _1475 = new _933();
    _1475._1094(_8287);
    function _8290(value)
    {
        return ((value & 0xFF) << 24) |
            ((value & 0xFF00) << 8) |
            ((value & 0xFF0000) >> 8) |
            ((value & 0xFF000000) >>> 24);
    }
    function _1101()
    {
        let _865;
        _865 = _1475._1101();
        _865 = _8290(_865);
        return _865;
    }

    
    let state = _1101();
    
    if (state != 0x5d9f2a6b)
    {
        alert(L("不是合法的拾音器插件，前面的魔术字不对 ") + state.toFixed(16));
        return false;
    }

    let _8291 = _1101();
    let _8292 = _1101();
    let _8293 = _1101();
    let _8294 = _1101();
    let _8295 = _1101();

    let _8296 = _1101();
    let _8297 = _1101();

    let _8298 = _1101();
    let _8299 = _1101();
    let _8300 = _1101();
    let _8301 = new Array(_8300);
    for (let _866 = 0; _866 < _8300; _866++)
        _8301[_866] = _1101();
    let _8302 = _1101();
    let _8303 = new Array(_8302);
    for (let _866 = 0; _866 < _8302; _866++)
        _8303[_866] = _1101();

    let _8304 = _1101();
    let _8305 = _1101();
    let _8306 = _1101();
    let _8307 = _1101();
    let _8308 = new Array(_8307);
    for (let _866 = 0; _866 < _8307; _866++)
        _8308[_866] = _1101();
    let _8309 = _1101();
    let _8310 = new Array(_8309);
    for (let _866 = 0; _866 < _8309; _866++)
        _8310[_866] = _1101();

    let code = new _933();
    _1475.position = _8298;
    _1475._1106(code, _8299);
    let data = new _933();
    _1475.position = _8304;
    _1475._1106(data, _8305);
    let _8311 = new _933();
    _1475.position = _8296;
    _1475._1106(_8311, _8297);

    let _8312 = Math.floor(_8299 / Uint32Array.BYTES_PER_ELEMENT);
    let _8313 = new Array(_8312);
    for (let _866 = 0; _866 < _8312; _866++)
        _8313[_866] = _8290(code._1101());
    let _8314 = new _933();
    code._1106(_8314, _8299 - _8312 * Uint32Array.BYTES_PER_ELEMENT)


    let _8315 = Math.floor(_8305 / Uint32Array.BYTES_PER_ELEMENT);
    let _8316 = new Array(_8315);
    for (let _866 = 0; _866 < _8315; _866++)
        _8316[_866] = _8290(data._1101());
    let _8317 = new _933();
    data._1106(_8317, _8305 - _8315 * Uint32Array.BYTES_PER_ELEMENT)

    
    if (_8299> 1024 * 128)
    {
        alert(L("插件太大，目前代码段只支持128K，代码段不够就没办法了"));
        return false;
    }
    if (_8305 > 1024 * 128)
    {
        alert(L("插件太大，目前数据段只支持128K，数据段不够可以动态申请"));
        return false;
    }
    for (let _866 = 0; _866 < _8300; _866++)
        _8313[_8301[_866] / Uint32Array.BYTES_PER_ELEMENT] += _8289;
    for (let _866 = 0; _866 < _8307; _866++)
        _8316[_8308[_866] / Uint32Array.BYTES_PER_ELEMENT] += _8289;
    for (let _866 = 0; _866 < _8302; _866++)
        _8313[_8303[_866] / Uint32Array.BYTES_PER_ELEMENT] += _8288;
    for (let _866 = 0; _866 < _8309; _866++)
        _8316[_8310[_866] / Uint32Array.BYTES_PER_ELEMENT] += _8288;

    
    for (let _934 = 0; _934 < 2; _934++)
    {
        if (_934 == 0)
            _8285._935();
        else
            _8285._936();

        function _1082(x)
        {
            x = _8290(x);
            _8285._1082(x);
        }

        _1082(state);

        _1082(_8291);
        _1082(_8292);
        _1082(_8293);
        _1082(_8294);
        _1082(_8295);

        let _8318;
        _8318 = _8285.position;
        _1082(0);
        _1082(_8297);

        let _8319;
        _8319 = _8285.position;
        _1082(0);
        _1082(_8299);
        let _8320;
        _8320 = _8285.position;
        _1082(0);
        _1082(_8305);
        _1082(_8306);

	    
	    
        
        const _8321 = 0x10000;
        let _8322;
        _8322 = Math.ceil(_8285.position / _8321) * _8321 - _8285.position;
        for (let _866 = 0; _866 < _8322; _866++)
            _8285._1079(0xcc);
        let _8323;
        _8323 = _8285.position;
        _8285.position = _8319;
        _1082(_8323);
        _8285.position = _8323;
        for (let _866 = 0; _866 < _8312; _866++)
            _1082(_8313[_866]);
        _8314.position = 0;
        _8285._1089(_8314);

        _8323 = _8285.position;
        _8285.position = _8320;
        _1082(_8323);
        _8285.position = _8323;
        for (let _866 = 0; _866 < _8315; _866++)
            _1082(_8316[_866]);
        _8317.position = 0;
        _8285._1089(_8317);

        _8323 = _8285.position;
        _8285.position = _8318;
        _1082(_8323);
        _8285.position = _8323;
        _8311.position = 0;
        _8285._1089(_8311);
    }

    _8285.position = 0;

    return true;
}
async function _8324(_8325, _8326)
{
    _667.disabled = true;
    _666.style.display = "none";
    _669.disabled = true;
    _668.style.display = "";
    let _8215;
    _8215 = _665.getElementsByTagName("div");
    for (let _866 = 0; _866 < _8215.length; _866++)
    {
        let _8216;
        _8216 = _8215[_866];
        _8216.setAttribute('onclick', null);
        _8216.setAttribute('ondblclick', null);
        if (_8216.style.backgroundColor == 'rgba(0, 0, 255, 0.2)')
            _8216.style.backgroundColor = 'rgba(0, 0, 0, 0.2)';
    }
    _669._8209 = true;
    _8197._8237();
    await _1170._8179(_8325, _8326);
}
async function _8327()
{
    _651._8209 = true;
    _8197._8237();
    await _1170._8178();
}


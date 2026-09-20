// [_730.js] class _8328 = 播放器: 空格键触发 _8329() 逐音符播放(_1214/_1215 双声道样本),
//   ScriptProcessor 直写输出, 16kHz, 单音符 64000 样本, 切换时 3200 样本线性淡出
class _8490
{
    _8491;
    _8492;
}
class _8328
{
    
    static _8493 = 0;
    static _8494 = new Array();
    static _8495(_8491, _8492)
    {
        let _8496 = new _8490();
        _8496._8491 = _8491;
        _8496._8492 = _8492;
        _8328._8494.push(_8496);
    }
    static _8497(_8491, _8492)
    {
        for (let _866 = 0; _866 < _8328._8494.length; _866++)
        {
            let _8496 = _8328._8494[_866];
            if (_8496._8491 == _8491 && _8496._8492 == _8492)
            {
                _8328._8494.splice(_866, 1);
                _866--;
                break;
            }
        }
    }

    
    static _8498 = null;

    
    static _8499() 
    {
        if (!_8328._8498)
        {
            _8328._8498 = new (window.AudioContext || window.webkitAudioContext)({ sampleRate: 16000 });
            _8328._8494.length = 0;
        }
        if (_8328._8498.state === 'suspended')
            _8328._8498.resume();
    }

    static _8500(_731, _8501)
    {
        if (_731==null || _731.length == 0)
            return;

        
        const _8492 = _8328._8498.createGain();
        _8492.gain.setValueAtTime(1, _8328._8498.currentTime);

        
        const _8491 = _8328._8498.createScriptProcessor(256, 1, 1);
        let _866 = 0;

        
        _8491.onaudioprocess = (_983) => 
        {
            const _1360 = _983.outputBuffer.getChannelData(0);
            for (let _1119 = 0; _1119 < _1360.length; _1119++) 
            {
                
                
                if (_866 < _731.length)
                {
                    _1360[_1119] = _731[_866];
                    _866++;
                }
                else
                    _1360[_1119] = 0;
            }
        };

        
        try 
        {
            _8491.connect(_8492);
            _8492.connect(_8328._8498.destination);
        }
        catch (_3503)
        {
            console.error('processor gain_node connect error：', _3503);
            return;
        }

        if (_8501)
            _8328._8495(_8491, _8492)

        setTimeout(() => 
        {
            try 
            {
                _8491.disconnect(_8492);
                _8492.disconnect(_8328._8498.destination);
                _8328._8497(_8491, _8492)
            }
            catch (_3503)
            {
                console.error('processor gain_node disconnect error：', _3503);
            }
        }, _1210._8486 * 1000 / _1202._1869);
    }
    static _8502()
    {
        for (let _866 = 0; _866 < _8328._8494.length; _866++)
        {
            let _8496 = _8328._8494[_866];
            let _8491, _8492;
            _8492 = _8496._8491;
            _8492 = _8496._8492;
            try 
            {

                const _8503 = _1210._8487 / _1202._1869; 
                _8492.gain.setValueAtTime(_8492.gain.value, _8328._8498.currentTime);
                _8492.gain.linearRampToValueAtTime(0, _8328._8498.currentTime + _8503);
            }
            catch (_3503)
            {
                console.error('processor gain_node disconnect error：', _3503);
            }
        }
    }
    static _8329() 
    {
        if (_8338.length == 0)
            return;
        _8328._8499();
        _8328._8493 %= _8338.length;
        let _1209 = _8338[_8328._8493];
        _8328._8493++;
        _8328._8493 %= _8338.length;
        _8328._8502();
        _8328._8500(_1209._1214, false);
        _8328._8500(_1209._1215, true);
    }

    static _8366()
    {
        _8328._8493 = 0;
    }
}


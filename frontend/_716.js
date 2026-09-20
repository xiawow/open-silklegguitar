// [_716.js] UI 锁框架: class _8197._8237() 在操作设备期间批量禁用控件, _8198() 恢复。
//   四种控件类型由 A("_9750".." _9753") 区分(页签/按钮/图标/动作图标)
class _8401
{
    _8402 = null;
    _8403 = "";
    _8404 = true;
    _8405 = false;

    _8406 = null;
    _8407 = "";
    constructor(_8408, _8409)
    {
        this._8402 = _8408;
        this._8403 = _8409;
    }
}
class _8197
{
    static _8410 = new Array();
    static _8411 = false;
    static _8336(_8408, _8409)
    {
        _8197._8410.push(new _8401(_8408, _8409));
    }
    static _8237(_2446)
    {
        if (_8197._8411)
            return;
        _8197._8411 = true;
        let _8412 = arguments;

        _697.disabled = false;
        _698.disabled = false;

        function _8413(_8408)
        {
            for (let _1059 = 0; _1059 < _8412.length; _1059++)
                if (_8412[_1059] == _8408)
                    return true;
            return false;
        }
        let _8414 = _8197._8410;
        for (let _1059 = 0; _1059 < _8414.length; _1059++)
        {
            let _3457 = _8414[_1059];
            if (_8413(_3457._8402))
            {
                _3457._8405 = true;
                continue;
            }
            _3457._8405 = false;
            switch (_3457._8403)
            {
                case A("_9751"):
                    _3457._8404 = !_3457._8402.disabled;
                    _3457._8406 = _3457._8402.onclick;
                    _3457._8407 = _3457._8402.style.cursor;
                    if (_3457._8404)
                    {
                        _3457._8402.disabled = true;
                        _3457._8402.onclick = null;
                        _3457._8402.style.cursor = "";
                    }
                    break;
                case A("_9752"):
                    if (_3457._8402.style.webkitFilter == "grayscale(100%)")
                        _3457._8404 = false;
                    else
                    {
                        _3457._8404 = true;
                        _3457._8406 = _3457._8402.onclick;
                        _3457._8407 = _3457._8402.style.cursor;
                        _3457._8402.style.webkitFilter = "grayscale(100%)";
                        _3457._8402.onclick = null;
                        _3457._8402.style.cursor = "";
                    }
                    break;
                case A("_9753"):
                    {
                        let _8415;
                        _8415 = _3457._8402.getAttribute("enable");
                        if (_8415 === null || _8415 == "true")
                            _3457._8404 = true;
                        else
                            _3457._8404 = false;
                        if (_3457._8404)
                            _878._879(_3457._8402, false);
                    }
                    break;
                case A("_9750"):
                    {
                        if (_3457._8402.onclick)
                            _3457._8404 = true;
                        else
                            _3457._8404 = false;
                        if (_3457._8404)
                            _3457._8402._8244();
                    }
                    break;
            }
        }
    }
    static _8198()
    {
        if (!_8197._8411)
            return;
        _8197._8411 = false;

        _697.disabled = true;
        _698.disabled = true;

        let _8414 = _8197._8410;
        for (let _1059 = 0; _1059 < _8414.length; _1059++)
        {
            let _3457 = _8414[_1059];
            if(_3457._8405)
                continue;
            if (!_3457._8404)
                continue;
            switch (_3457._8403)
            {
                case A("_9751"):
                    _3457._8402.onclick = _3457._8406;
                    _3457._8402.style.cursor = _3457._8407;
                    _3457._8402.disabled = false;;
                    break;
                case A("_9752"):
                    _3457._8402.onclick = _3457._8406;
                    _3457._8402.style.cursor = _3457._8407;
                    _3457._8402.style.webkitFilter = "";
                    break;
                case A("_9753"):
                    _878._879(_3457._8402, true);
                    break;
                case A("_9750"):
                    _3457._8402._1346();
                    break;
            }
        }
    }
}

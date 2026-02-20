import numpy as np

from c_business_layer.kpi_business import KpiBusiness


def main() -> None:
    kb = KpiBusiness()
    res = kb.run_pair("hdf_db/input.h5", "hdf_db/output.h5", title="debug")
    store = res["store"]

    sensor = "CEER_FL"
    common, overall, _per_param = kb._compute_match_pct(store, sensor)

    print("sensor:", sensor)
    print("common scans:", len(common))
    if len(common) == 0:
        return

    in_scan = store.get_scan_index("input", sensor)
    out_scan = store.get_scan_index("output", sensor)
    in_map = {int(v): i for i, v in enumerate(in_scan)}
    out_map = {int(v): i for i, v in enumerate(out_scan)}

    in_hdr = store.get_header_signals("input", sensor).get(
        "HED_NUM_OF_VALID_DETECTIONS"
    )
    out_hdr = store.get_header_signals("output", sensor).get(
        "HED_NUM_OF_VALID_DETECTIONS"
    )

    idxs = [0, min(10, len(common) - 1), len(common) - 1]
    for j in idxs:
        s = int(common[j])
        in_r = in_map[s]
        out_r = out_map[s]
        in_n = int(np.rint(in_hdr[in_r])) if in_hdr is not None else None
        out_n = int(np.rint(out_hdr[out_r])) if out_hdr is not None else None
        print(
            f"scan={s} rows(in,out)=({in_r},{out_r}) N(in,out)=({in_n},{out_n}) overall%={float(overall[j])}"
        )

    # Recompute the first scan's matched_all to sanity-check denominator usage.
    s0 = int(common[0])
    in_r0 = in_map[s0]
    out_r0 = out_map[s0]
    n_det = int(min(int(np.rint(in_hdr[in_r0])), int(np.rint(out_hdr[out_r0]))))

    in_sig = {
        sig: store.get_detection_signal("input", sensor, sig) or {}
        for sig in kb.MATCH_SIGNALS
    }
    out_sig = {
        sig: store.get_detection_signal("output", sensor, sig) or {}
        for sig in kb.MATCH_SIGNALS
    }

    matched_all = 0
    for det_idx in range(1, n_det + 1):
        rin_arr = in_sig["DET_RANGE"].get(det_idx)
        rout_arr = out_sig["DET_RANGE"].get(det_idx)
        if rin_arr is None or rout_arr is None:
            continue
        if in_r0 >= len(rin_arr) or out_r0 >= len(rout_arr):
            continue

        rin = float(rin_arr[in_r0])
        rout = float(rout_arr[out_r0])
        if np.isnan(rin) or np.isnan(rout) or rin == 0.0 or rout == 0.0:
            continue

        det_all_ok = True
        for sig in kb.MATCH_SIGNALS:
            ain_arr = in_sig[sig].get(det_idx)
            aout_arr = out_sig[sig].get(det_idx)
            if ain_arr is None or aout_arr is None:
                det_all_ok = False
                continue
            if in_r0 >= len(ain_arr) or out_r0 >= len(aout_arr):
                det_all_ok = False
                continue

            ain = round(float(ain_arr[in_r0]), 2)
            aout = round(float(aout_arr[out_r0]), 2)
            if np.isnan(ain) or np.isnan(aout):
                det_all_ok = False
                continue
            if abs(ain - aout) > float(kb.MATCH_TOLERANCES[sig]):
                det_all_ok = False

        if det_all_ok:
            matched_all += 1

    print(
        f"first scan recompute: scan={s0} n_det={n_det} matched_all={matched_all} pct={100.0 * matched_all / n_det:.2f}"
    )


if __name__ == "__main__":
    main()

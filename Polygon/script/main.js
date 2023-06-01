/// <reference path="math.js" />
/// <reference path="drawer.js" />
const WIDTH = 320;
const HEIGHT = 320;

let gDrawer = new Drawer("graph", WIDTH*2, HEIGHT*2);
let gDragIndex = -1;
let gStep = -1;
/** @type{vec[]} */
let gVertList;
/** @type{{ distance : number, deleted : boolean }[]} */
let gVertInfo;

initVert(5);
requestNextAnimationFrame(main);

function main() {
	gDrawer.clear();
	loop();
	requestNextAnimationFrame(main);
}
function loop() {
	if (gDrawer.isDrag) {
		for(let i=0; i<gVertList.length && gDragIndex < 0; i++) {
			if (distance(gDrawer.cursor, gVertList[i]) <= 10) {
				gDragIndex = i;
				break;
			}
		}
	} else {
		gDragIndex = -1;
	}

	if (0 <= gDragIndex) {
		gDrawer.cursor.copy(gVertList[gDragIndex]);
        gStep = -1;
	}
	for(let i=0; i<gVertList.length; i++) {
		let p = new vec();
		gVertList[i].copy(p);
		p.X += 10000;
		p.Y -= 10000;
		gVertInfo[i].distance = p.abs;
		gVertInfo[i].deleted = false;
	}

	gDrawer.drawPolyline(gVertList, [0,0,0]);
	gDrawer.drawLine(gVertList[gVertList.length-1], gVertList[0], [0,0,0]);

	draw_polygon(1, gStep);
}
function btnReset_onclick(obj) {
    if (-1 == gStep) {
        initVert(gVertList.length);
    }
    gStep = -1;
}
function btnPrev_onclick(obj) {
    if (0 <= gStep) {
        gStep--;
    }
}
function btnNext_onclick(obj) {
    gStep++;
}
function txtVerts_onchange(obj) {
    if (Number.isNaN(obj.value * 1)) {
        obj.value = 3;
    }
    let verts = obj.value * 1;
    if (verts < 3) {
        verts = 3;
    }
    initVert(verts);
    gStep = -1;
}

function initVert(vertCount) {
    gVertList = new Array();
    for (let i=0; i<vertCount; i++) {
        let th = -2*Math.PI*i/vertCount;
        let x = Math.cos(th) * 0.9 * HEIGHT / 2 + HEIGHT;
        let y = Math.sin(th) * 0.9 * HEIGHT / 2 - HEIGHT;
        gVertList.push(new vec(x, y, 0));
    }
    gVertInfo = new Array();
    for (let i=0; i<gVertList.length; i++) {
        gVertInfo.push({distance: 0, deleted: false});
    }
}

/**
 * @param {vec} va 
 * @param {vec} vo 
 * @param {vec} vb 
 * @param {vec} p 
 * @returns {boolean}
 */
function inner_triangle(va, vo, vb, p) {
    let oq_x = va.X - vb.X;
    let oq_y = va.Y - vb.Y;
    let op_x = p.X - vb.X;
    let op_y = p.Y - vb.Y;
    let normal_abp = oq_x * op_y - oq_y * op_x;
    oq_x = vo.X - va.X;
    oq_y = vo.Y - va.Y;
    op_x = p.X - va.X;
    op_y = p.Y - va.Y;
    let normal_oap = oq_x * op_y - oq_y * op_x;
    oq_x = vb.X - vo.X;
    oq_y = vb.Y - vo.Y;
    op_x = p.X - vo.X;
    op_y = p.Y - vo.Y;
    let normal_bop = oq_x * op_y - oq_y * op_x;
    if (normal_abp < 0 && normal_oap < 0 && normal_bop < 0) {
        return true;
    }
    if (normal_abp > 0 && normal_oap > 0 && normal_bop > 0) {
        return true;
    }
    return false;
}

/**
 * @param {number} order 
 * @param {number} max_step 
 */
function draw_polygon(order, max_step) {
	const INDEX_COUNT = gVertList.length;
    const INDEX_NEXT = INDEX_COUNT + order;
    const INDEX_RIGHT = 1;
    const INDEX_LEFT = INDEX_COUNT - 1;
	let reverse_count = 0;
	let vert_count = 0;
	let surf_list = new Array();
    let tri_move = {ia:-1, io:-1, ib:-1};
    let tri_far = {ia:0, io:0, ib:0};
	let step = 0;
	do { // 最も遠くにある頂点(vo)の取得ループ
        /*** 最も遠くにある頂点(vo)を取得 ***/
        let vo, io = 0;
        let dist_max = 0.0;
		vert_count = 0;
		for (let i = 0; i < INDEX_COUNT; i++) {
			let info = gVertInfo[i];
			if (info.deleted) {
				continue;
			}
			if (dist_max < info.distance) {
				dist_max = info.distance;
				io = i;
			}
			vert_count++;
		}
		reverse_count = 0;
        vo = gVertList[io];

        {
            /*** 頂点(vo)の左隣にある頂点(va)を取得 ***/
            let ia = (io + INDEX_LEFT) % INDEX_COUNT;
            for (let i = 0; i < INDEX_COUNT; i++) {
                if (gVertInfo[ia].deleted) {
                    ia = (ia + INDEX_LEFT) % INDEX_COUNT;
                } else {
                    break;
                }
            }
            /*** 頂点(vo)の右隣にある頂点(vb)を取得 ***/
            let ib = (io + INDEX_RIGHT) % INDEX_COUNT;
            for (let i = 0; i < INDEX_COUNT; i++) {
                if (gVertInfo[ib].deleted) {
                    ib = (ib + INDEX_RIGHT) % INDEX_COUNT;
                } else {
                    break;
                }
            }
            tri_far.ia = ia;
            tri_far.io = io;
            tri_far.ib = ib;
        }

        if (max_step < ++step) {
            break;
        }
        while (true) { // 頂点(vo)の移動ループ
            /*** 頂点(vo)の左隣にある頂点(va)を取得 ***/
            let ia = (io + INDEX_LEFT) % INDEX_COUNT;
            for (let i = 0; i < INDEX_COUNT; i++) {
                if (gVertInfo[ia].deleted) {
                    ia = (ia + INDEX_LEFT) % INDEX_COUNT;
                } else {
                    break;
                }
            }
            let va = gVertList[ia];
            /*** 頂点(vo)の右隣にある頂点(vb)を取得 ***/
            let ib = (io + INDEX_RIGHT) % INDEX_COUNT;
            for (let i = 0; i < INDEX_COUNT; i++) {
                if (gVertInfo[ib].deleted) {
                    ib = (ib + INDEX_RIGHT) % INDEX_COUNT;
                } else {
                    break;
                }
            }
            let vb = gVertList[ib];
            tri_move.ia = ia;
            tri_move.ib = ib;
            /*** 三角形(va vo vb)の表裏を確認 ***/
            let aob_normal = 0;
            let oa_x = va.X - vo.X;
            let oa_y = va.Y - vo.Y;
            let ob_x = vb.X - vo.X;
            let ob_y = vb.Y - vo.Y;
            aob_normal = (oa_x * ob_y - oa_y * ob_x) * order;
            if (aob_normal < 0) {
                /*** 裏の場合 ***/
                reverse_count++;
                if (INDEX_COUNT < reverse_count) {
                    /*** 表になる三角形(va vo vb)がない場合 ***/
                    /*** 頂点(vo)を検索対象から削除 ***/
                    gVertInfo[io].deleted = true;
                    /*** 次の最も遠くにある頂点(vo)を取得 ***/
                    break;
                }
                /*** 頂点(vo)を隣に移動 ***/
                io = (io + INDEX_NEXT) % INDEX_COUNT;
                for (let i = 0; i < INDEX_COUNT; i++) {
                    if (gVertInfo[io].deleted) {
                        io = (io + INDEX_NEXT) % INDEX_COUNT;
                    } else {
                        break;
                    }
                }
                vo = gVertList[io];
                {
                    /*** 頂点(vo)の左隣にある頂点(va)を取得 ***/
                    ia = (io + INDEX_LEFT) % INDEX_COUNT;
                    for (let i = 0; i < INDEX_COUNT; i++) {
                        if (gVertInfo[ia].deleted) {
                            ia = (ia + INDEX_LEFT) % INDEX_COUNT;
                        } else {
                            break;
                        }
                    }
                    /*** 頂点(vo)の右隣にある頂点(vb)を取得 ***/
                    ib = (io + INDEX_RIGHT) % INDEX_COUNT;
                    for (let i = 0; i < INDEX_COUNT; i++) {
                        if (gVertInfo[ib].deleted) {
                            ib = (ib + INDEX_RIGHT) % INDEX_COUNT;
                        } else {
                            break;
                        }
                    }
                    tri_move.ia = ia;
                    tri_move.ib = ib;
                    tri_move.io = io;
                }
                if (max_step < ++step) {
                    break;
                }
                continue;
            }
            /*** 三角形(va vo vb)の内側にva vo vb以外の頂点がないか確認 ***/
            let point_in_triangle = false;
            for (let i = 0; i < INDEX_COUNT; i++) {
                if (i == ia || i == io || i == ib || gVertInfo[i].deleted) {
                    continue;
                }
                let p = gVertList[i];
                if (inner_triangle(va, vo, vb, p)) {
                    point_in_triangle = true;
                    break;
                }
            }
            if (point_in_triangle) {
                /*** 内側に他の頂点がある場合 ***/
                /*** 頂点(vo)を隣に移動 ***/
                io = (io + INDEX_NEXT) % INDEX_COUNT;
                for (let i = 0; i < INDEX_COUNT; i++) {
                    if (gVertInfo[io].deleted) {
                        io = (io + INDEX_NEXT) % INDEX_COUNT;
                    } else {
                        break;
                    }
                }
                vo = gVertList[io];
                {
                    /*** 頂点(vo)の左隣にある頂点(va)を取得 ***/
                    ia = (io + INDEX_LEFT) % INDEX_COUNT;
                    for (let i = 0; i < INDEX_COUNT; i++) {
                        if (gVertInfo[ia].deleted) {
                            ia = (ia + INDEX_LEFT) % INDEX_COUNT;
                        } else {
                            break;
                        }
                    }
                    /*** 頂点(vo)の右隣にある頂点(vb)を取得 ***/
                    ib = (io + INDEX_RIGHT) % INDEX_COUNT;
                    for (let i = 0; i < INDEX_COUNT; i++) {
                        if (gVertInfo[ib].deleted) {
                            ib = (ib + INDEX_RIGHT) % INDEX_COUNT;
                        } else {
                            break;
                        }
                    }
                    tri_move.ia = ia;
                    tri_move.ib = ib;
                    tri_move.io = io;
                }
                if (max_step < ++step) {
                    break;
                }
            } else {
                /*** 内側に他の頂点がない場合 ***/
                /*** 三角形(va vo vb)を面リストに追加 ***/
                let surf = [
                    gVertList[ia],
                    gVertList[io],
                    gVertList[ib],
                    gVertList[ia]
                ];
                surf_list.push(surf);
                /*** 頂点(vo)を検索対象から削除 ***/
                gVertInfo[io].deleted = true;
                /*** 次の最も遠くにある頂点(vo)を取得 ***/
                break;
            }
        } // 頂点(vo)の移動ループ
        if (max_step < step) {
            break;
        }
    } while (3 < vert_count); // 最も遠くにある頂点(vo)の取得ループ

    for(let i=0; i<surf_list.length; i++) {
        gDrawer.fillPolygon(surf_list[i], new vec(), [211,211,211]);
        gDrawer.drawPolyline(surf_list[i], [0,0,255], 1);
    }
    for(let i=0; i<gVertList.length; i++) {
        let v = gVertList[i];
        if (gVertInfo[i].deleted) {
            gDrawer.fillCircle(v, 4, [211, 211, 211]);
            gDrawer.drawCircle(v, 4);
        } else {
            gDrawer.fillCircle(v, 9);
        }
    }

    if (max_step < 0) {
        return;
    }

    let drawMove = 0 <= tri_move.io && !gVertInfo[tri_move.io].deleted;
    if (drawMove) {
        let a = gVertList[tri_move.ia];
        let o = gVertList[tri_move.io];
        let b = gVertList[tri_move.ib];
        gDrawer.drawArrowC(a, o, [0,0,0], 3);
        gDrawer.drawArrowC(o, b, [0,0,0], 3);
        gDrawer.drawArrowC(b, a, [0,0,0], 3);
        gDrawer.fillCircle(o, 9, [0,255,0]);
        gDrawer.drawCircle(o, 9);
    }
    let drawFar = !gVertInfo[tri_far.io].deleted;
    if (drawFar) {
        let a = gVertList[tri_far.ia];
        let o = gVertList[tri_far.io];
        let b = gVertList[tri_far.ib];
        if (!drawMove) {
            gDrawer.drawArrowC(a, o, [0,0,0], 3);
            gDrawer.drawArrowC(o, b, [0,0,0], 3);
            gDrawer.drawArrowC(b, a, [0,0,0], 3);
        }
        gDrawer.fillCircle(o, 9, [231,231,0]);
        gDrawer.drawCircle(o, 9);
        if (drawMove) {
            a = gVertList[tri_move.ia];
            o = gVertList[tri_move.io];
            b = gVertList[tri_move.ib];
            gDrawer.drawStringXY(o.X - 5, o.Y - 5, "O", 16);
            if (tri_move.ia == tri_far.io) {
                gDrawer.drawStringXY(a.X - 5, a.Y - 4, "a", 16);
            } else {
                gDrawer.drawStringXY(a.X - 5, a.Y - 4, "a", 16, [0,255,255]);
            }
            if (tri_move.ib == tri_far.io) {
                gDrawer.drawStringXY(b.X - 5, b.Y - 5, "b", 16);
            } else {
                gDrawer.drawStringXY(b.X - 5, b.Y - 5, "b", 16, [0,255,255]);
            }
        } else {
            gDrawer.drawStringXY(a.X - 5, a.Y - 4, "a", 16, [0,255,255]);
            gDrawer.drawStringXY(b.X - 5, b.Y - 5, "b", 16, [0,255,255]);
            gDrawer.drawStringXY(o.X - 5, o.Y - 5, "O", 16);
        }
    }
}
/*
 * Copyright 2020-present Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

var assert = require('assert');
var helper = require('./support/helper');
helper.loadSqlite3();

let q1 = `
SELECT BB.SEQ_NO, AA.PAY_TYPE_FG
        , ( SELECT
                CASE
                    WHEN '09' = AA.PAY_TYPE_FG THEN
                        CASE WHEN 'M' = ? THEN '회원마일리지'
                             ELSE '회원스탬프금액'
                        END
                    ELSE COM_CD_NM
                END AS COM_CD_NM
            FROM CCD_CODEM_T
            WHERE COM_CD_FG= '038'
                AND COM_CD = AA.PAY_TYPE_FG
                AND USE_YN = 'Y') AS PAY_TYPE_NM
        , CASE AA.SALE_YN WHEN 'Y' THEN '정상' ELSE '취소' END AS STATUS
        , AA.PAY_AMT, COALESCE( BB.LINE_NO, '' ) AS LINE_NO
        , BB.CORNER_CD, BB.VAN_TERM_NO, BB.APPR_REQ_AMT, BB.APPR_AMT, BB.VAT_AMT, BB.SVC_TIP_AMT, BB.APPR_TYPE_FG
        , BB.APPR_FG, BB.APPR_PROC_FG, BB.CARD_IN_FG, BB.APPR_DATE, BB.APPR_NO, BB.APPR_IDT_TYPE, BB.APPR_IDT_FG
        , BB.CARD_NO, BB.INST_MM_FG, BB.INST_MM_CNT, BB.VALID_TERM, BB.SIGN_PAD_YN, BB.CRDCP_CD, BB.APPR_DC_AMT
        , BB.APPR_LOG_NO, BB.JCD_PROC_FG, BB.JCD_TYPE_FG, BB.JCD_PAY_FG, BB.CNMK_CD, BB.UNION_PAY_FG, BB.USER_DATA
 FROM SSL_TRPSQ_T AA
        INNER JOIN ( SELECT SHOP_CD, SALE_DATE, POS_NO, BILL_NO, LINE_NO
                                 , SEQ_NO, '02' AS PAY_TYPE_FG, SALE_YN, CORNER_CD
                                 , VAN_TERM_NO, APPR_REQ_AMT, APPR_AMT, VAT_AMT
                                 , SVC_TIP_AMT, '0' AS APPR_TYPE_FG
                                 , SALE_YN AS APPR_FG
                                 , APPR_PROC_FG, CARD_IN_FG, APPR_DATE, APPR_NO, '' AS APPR_IDT_TYPE, '' AS APPR_IDT_FG, CRD_CARD_NO AS CARD_NO
                                 , INST_MM_FG, INST_MM_CNT, VALID_TERM, SIGN_PAD_YN, CRDCP_CD, APPR_DC_AMT, APPR_LOG_NO, '' AS JCD_PROC_FG
                                 , '' AS JCD_TYPE_FG, '' AS JCD_PAY_FG, CNMK_CD, UNION_PAY_FG, '' AS USER_DATA
                          FROM SSL_TRCRD_T
                         WHERE SHOP_CD = ?
                            AND SALE_DATE = ?
                            AND POS_NO = ?
                            AND APPR_PROC_FG <> '0'
                    UNION ALL
                         SELECT SHOP_CD, SALE_DATE, POS_NO, BILL_NO, LINE_NO, SEQ_NO, '01' AS PAY_TYPE_FG, SALE_YN, CORNER_CD, VAN_TERM_NO
                                  , 0 AS APPR_REQ_AMT, APPR_AMT, VAT_AMT, SVC_TIP_AMT, '0' AS APPR_TYPE_FG, SALE_YN AS APPR_FG, APPR_PROC_FG
                                  , CARD_IN_FG, APPR_DATE, APPR_NO, APPR_IDT_TYPE, APPR_IDT_FG, APPR_IDT_NO AS CARD_NO, '' AS INST_MM_FG
                                  , 0 AS INST_MM_CNT, '' AS VALID_TERM, '' AS SIGN_PAD_YN, '' AS CRDCP_CD, 0 AS APPR_DC_AMT, APPR_LOG_NO
                                  , '' AS JCD_PROC_FG, '' AS JCD_TYPE_FG, '' AS JCD_PAY_FG, CNMK_CD, '0' AS UNION_PAY_FG, '' AS USER_DATA
                          FROM SSL_TRCSH_T
                         WHERE SHOP_CD = ?
                            AND SALE_DATE = ?
                            AND POS_NO = ?
                            AND APPR_PROC_FG <> '0'
                            AND COALESCE( APPR_NO,'' ) <> ''
                   UNION ALL
                         SELECT SHOP_CD, SALE_DATE, POS_NO, BILL_NO, LINE_NO, SEQ_NO
                                 , CASE JCD_PROC_FG WHEN '0' THEN '04' WHEN '1' THEN '07' WHEN '2' THEN '03' END AS PAY_TYPE_FG
                                 , SALE_YN, CORNER_CD, VAN_TERM_NO, 0 AS APPR_REQ_AMT, APPR_AMT, 0 AS VAT_AMT
                                 , 0 AS SVC_TIP_AMT, '0' AS APPR_TYPE_FG, SALE_YN AS APPR_FG, APPR_PROC_FG
                                 , CARD_IN_FG, APPR_DATE, APPR_NO, '' AS APPR_IDT_TYPE, '' AS APPR_IDT_FG, JCD_CARD_NO AS CARD_NO
                                 , '' AS INST_MM_FG, 0 AS INST_MM_CNT, VALID_TERM
                                 , SIGN_PAD_YN, '' AS CRDCP_CD, 0 AS APPR_DC_AMT, APPR_LOG_NO, JCD_PROC_FG, JCD_TYPE_FG, JCD_PAY_FG
                                 , CNMK_CD , '0' AS UNION_PAY_FG, '' AS USER_DATA
                          FROM SSL_TRJCD_T
                         WHERE SHOP_CD = ? AND SALE_DATE = ? AND POS_NO = ? AND APPR_PROC_FG <> '0' AND JCD_CD <> 'ZBNK' AND   JCD_PROC_FG  IN ('0','1','2')
                   UNION ALL
                        SELECT SHOP_CD, SALE_DATE, POS_NO, BILL_NO, LINE_NO, SEQ_NO, '11' AS PAY_TYPE_FG, SALE_YN, CORNER_CD, 0 AS VAN_TERM_NO
                                , APPR_AMT AS APPR_REQ_AMT, APPR_AMT, VAT_AMT, SVC_TIP_AMT, '0' AS APPR_TYPE_FG, SALE_YN AS APPR_FG, APPR_PROC_FG
                                , 'K' AS CARD_IN_FG, APPR_DATE, APPR_NO, '' AS APPR_IDT_TYPE, '' AS APPR_IDT_FG, CARD_NO, '' AS INST_MM_FG, 0 AS INST_MM_CNT
                                , '' AS VALID_TERM, '' AS SIGN_PAD_YN, '' AS CRDCP_CD, 0 AS APPR_DC_AMT, '' APPR_LOG_NO, '' AS JCD_PROC_FG, '' AS JCD_TYPE_FG
                                , '' AS JCD_PAY_FG, '' AS CNMK_CD, '0' AS UNION_PAY_FG, USER_DATA
                         FROM SSL_TRETC_T
                        WHERE SHOP_CD = ? AND SALE_DATE = ? AND POS_NO = ? AND APPR_PROC_FG <> '0'
                           AND TR_TYPE_CD IN ('011', '021', '041', '042', '051') ) BB
                ON BB.SHOP_CD = AA.SHOP_CD
              AND BB.SALE_DATE = AA.SALE_DATE
              AND BB.POS_NO = AA.POS_NO
              AND BB.BILL_NO = AA.BILL_NO
              AND BB.LINE_NO = AA.LINE_NO
              AND BB.PAY_TYPE_FG = AA.PAY_TYPE_FG
WHERE AA.SHOP_CD = ?
   AND AA.SALE_DATE = ?
   AND AA.POS_NO = ?
   AND AA.PAY_SEQ_NO = ?
   AND BB.APPR_PROC_FG <> '0'
ORDER BY AA.PAY_TYPE_FG, AA.PAY_SEQ_NO;
`;

let q2 = `
SELECT REGIS.SALE_DATE
     , REGIS.CLOSE_DT
     , REGIS.REGI_SEQ
     , REGIS.EMP_NO
     , EMPMS.EMP_NM
FROM POS_REGIS_T REGIS
     LEFT OUTER JOIN SCD_EMPMS_T EMPMS
      ON EMPMS.SHOP_CD = REGIS.SHOP_CD
     AND EMPMS.EMP_NO  = REGIS.EMP_NO
WHERE REGIS.SHOP_CD   = ?
  AND REGIS.SALE_DATE = (SELECT SALE_DATE
                           FROM POS_REGIS_T
                          WHERE SHOP_CD  = ?
                            AND POS_NO   = ?
                            AND REGI_SEQ = '00'
                          ORDER BY SALE_DATE DESC
                          LIMIT 1)
  AND REGIS.POS_NO    = ?
/*
  AND REGIS.CLOSE_FG  = (SELECT CASE WHEN POS_NO == '01' THEN 3 ELSE 2 END
                           FROM SCD_ENVPS_T
                          WHERE SHOP_CD     = ?
                            AND ENV_SET_CD  = '205'
                            AND ENV_SET_VAL = '1')
*/
ORDER BY REGIS.SALE_DATE DESC
    , REGIS.REGI_SEQ DESC ;
`;

let q3 = `
SELECT SALE_DATE  AS SALE_DATE
     , SUM(CASE WHEN SALE_YN = 'Y' THEN  1 ELSE -1 END  * SALE_QTY ) AS SALE_QTY
     , SUM(CASE WHEN SALE_YN = 'Y' THEN  1 ELSE -1 END  * SALE_AMT ) AS SALE_AMT
     , SUM(CASE WHEN SALE_YN = 'Y' THEN  1 ELSE -1 END  * DC_AMT ) AS SALE_DC
     , SUM(CASE WHEN SALE_YN = 'Y' THEN  1 ELSE -1 END  * DCM_SALE_AMT ) AS SALE_DCM
     , SUM(CASE WHEN SALE_YN = 'Y' THEN  1 ELSE -1 END  * VAT_AMT ) AS SALE_VAT
FROM SSL_TRDTL_T
WHERE SHOP_CD = ?
  AND POS_NO  = ?
GROUP BY SALE_DATE;
`;

let q4 = `
SELECT MS.ENV_SET_CD   AS ENV_SET_CD
     , MS.ENV_SET_VAL  AS ENV_SET_VAL
     , MS.ENV_SET_NM   AS ENV_SET_NM
     , DT.ENV_VAL_NM   AS ENV_VAL_NM
  FROM ( SELECT HD.ENV_SET_CD
            , HD.ENV_SET_NM
            , SH.ENV_SET_VAL
       FROM ( SELECT ENV_SET_CD
                   , ENV_SET_NM
              FROM CCD_ENVHD_T
             WHERE ENV_SET_FG IN ( '0','1','2', '5' )
              AND   USE_YN     = 'Y' ) HD
            LEFT OUTER JOIN SCD_ENVSH_T SH
            ON  SH.SHOP_CD    = ?
            AND SH.USE_YN     = 'Y'
            AND SH.ENV_SET_CD = HD.ENV_SET_CD ) MS
     LEFT OUTER JOIN ( SELECT ENV_SET_CD
                            , ENV_VAL_CD
                            , ENV_VAL_NM
                       FROM CCD_ENVDT_T ) DT
     ON  DT.ENV_SET_CD = MS.ENV_SET_CD
     AND DT.ENV_VAL_CD = MS.ENV_SET_VAL
ORDER BY MS.ENV_SET_CD;
`;

let q5 = `
SELECT
	CASE
		WHEN '1' = VAT_CD AND TAX_YN = 'Y' THEN SALE_UPRC - 공급가
		WHEN '0' = VAT_CD AND TAX_YN = 'Y' THEN CAST ( ROUND( (   공급가 * 0.1 ) ) AS INTEGER) /** 과세 품목. 미포함 **/
		WHEN TAX_YN = 'N'                  THEN 0 /** 비과세 품목.**/
		ELSE 0
	 END  VAT
	,  *
 FROM (
  SELECT  DISTINCT
          A.TU_FG                           AS TU_FG
        , A.TU_CLS_CD                       AS TU_CLS_CD
        , A.TU_KEY_CD                       AS TU_KEY_CD
        , A.PROD_CD                         AS PROD_CD
        , B.PROD_NM                         AS PROD_NM
        , B.TAX_YN                          AS TAX_YN
        , IFNULL( B.PROD_NM_EN , B.PROD_NM) AS PROD_NM_EN
        , B.IMG_URL                         AS IMG_URL
        , B.IMG_DOWNLOAD_FG                 AS IMG_DOWNLOAD_FG
        , B.KIOSK_PROD_STATUS               AS KIOSK_PROD_STATUS
        , C.SALE_UPRC                       AS SALE_UPRC
		    /** 과세 포함.**/
		   , CASE
				WHEN '1' = VAT_CD AND TAX_YN = 'Y' THEN CAST ( ROUND( ( C.SALE_UPRC / 1.1 ) )  AS INTEGER)  /** 부가세 포함,과세 품목.**/
				WHEN '0' = VAT_CD AND TAX_YN = 'Y' THEN C.SALE_UPRC                                         /** 부가세 비포함,과세 품목.**/
				WHEN TAX_YN = 'N'                  THEN C.SALE_UPRC                                         /** 비과세 품목.**/
		       ELSE 0
		      END AS  공급가
        , B.SIDE_MENU_YN                     AS SIDE_MENU_YN /** 사이드메뉴 사용여부      **/
        , B.SDA_CLS_CD                       AS SDA_CLS_CD   /** 사이드메뉴-속성분류코드   **/
        , B.SDS_GRP_CD                       AS SDS_GRP_CD   /** 사이드메뉴-선택그룹코드   **/
        , 1                                  AS ORDER_QTY    /** 주문수량에 사용하기 위함  **/
        , STAMP_ACC_YN
        , STAMP_ACC_QTY
        , STAMP_USE_YN
        , STAMP_USE_QTY
        , VAT_CD
     FROM SCD_KTKEY_T A   /** KIOSK 터치상품 **/
        , SCD_PRODM_T B   /** 상품마스터      **/
  	  , (
  		  SELECT
  		      A.PROD_CD
  		    , A.S_DATE
  		    , A.E_DATE
  		    , A.SALE_UPRC
  		  FROM SCD_PRCDT_T  A
  		    , (
  		  	SELECT
  		  	  PROD_CD, MIN(S_DATE) S_DATE
  		  	FROM SCD_PRCDT_T
  		  	WHERE SHOP_CD = ? /* shop_cd */
  		  	AND S_DATE  <= strftime('%Y%m%d' , 'NOW' , 'localtime')
  		  	AND E_DATE  >= strftime('%Y%m%d' , 'NOW' , 'localtime')
  		  	GROUP BY PROD_CD
  		    ) B
  		  WHERE A.SHOP_CD = ? /* shop_cd */
  		    AND A.S_DATE  <= strftime('%Y%m%d' , 'NOW' , 'localtime')
  		    AND A.E_DATE  >= strftime('%Y%m%d' , 'NOW' , 'localtime')
  		    AND A.PROD_CD = B.PROD_CD
  		    AND A.S_DATE  = B.S_DATE
  	   ) C
	   , (SELECT  1 AS VAT_CD ) D /** VAT 포함 여부 코드 **/ /* VAT CD (CODE 128)  1 : 과세 포함, 0 : 과세 불포함. */
   WHERE B.SHOP_CD       = A.SHOP_CD
     AND B.PROD_CD       = A.PROD_CD
     AND A.SHOP_CD       = ? /* shop_cd */
     /* AND A.TU_FG         = 'S' */
     /* AND A.TU_CLS_CD     = IFNULL ( ? , A.TU_CLS_CD ) */ /* clscd-> 터치분류CD */
     AND B.PRICE_MGR_FG  != '1'
     AND B.USE_YN        = 'Y'
     AND B.KIOSK_PROD_YN = 'Y'  /** KIOSK 상품 여부 **/
     AND B.SALE_PROD_YN  = 'Y'
     AND A.PROD_CD       = C.PROD_CD
);
`;

describe('kiosk sample queries', function () {
  let filename = 'skiosk.db';
  let db;

  before(function () {
  });

  it('+ve: should open the database', async () => {
    db = await new sqlite3.Database(helper.getStoreFilePath(filename));
  });

  // Get SHOP_CD, POS_NO, ... from SSL_TRHDR_T
  it('+ve: query SSL_TRHDR_T', async () => {
    let counter = 0;
    let count = await db.each('select * from SSL_TRHDR_T', (row) => {
      assert(row !== null);
      counter++;
    });
    assert(count === 173);
    assert(count === counter);
  });

  it('+ve: query q1', async () => {
    let counter = 0;
    let count = await db.each(q1, '0',
                              '031765', '20200909', '02',
                              '031765', '20200909', '02',
                              '031765', '20200909', '02',
                              '031765', '20200909', '02',
                              '031765', '20200909', '02',
                              '01', (row) => {
      assert(row !== null);
      counter++;
    });
    assert(count === 8);
    assert(count === counter);
  });

  it('+ve: query POS_REGIS_T', async () => {
    let counter = 0;
    let count = await db.each('select * from POS_REGIS_T', (row) => {
      assert(row !== null);
      counter++;
    });
    assert(count === 21);
    assert(count === counter);
  });

  it('+ve: query q2', async () => {
    let counter = 0;
    let count = await db.each(q2, '031765', '031765', '02', '02', //'031765'
      (row) => {
        assert(row !== null);
        counter++;
      });
    assert(count === 15);
    assert(count === counter);
  });

  it('+ve: query q3', async () => {
    let counter = 0;
    let count = await db.each(q3, '031765', '02', (row) => {
      assert(row !== null);
      counter++;
    });
    assert(count === 3);
    assert(count === counter);
  });

  it('+ve: query q4', async () => {
    let counter = 0;
    let count = await db.each(q4, '031765', (row) => {
      assert(row !== null);
      counter++;
    });
    assert(count === 203);
    assert(count === counter);
  });

  if (!helper.isTizen) {
  it('+ve: query q5', async () => {
    // NOTE: libsqlite3 on Tizen 4.0 cannot run this query
    let counter = 0;
    let count = await db.each(q5, '031765', '031765', '031765', (row) => {
      assert(row !== null);
      counter++;
    });
    assert(count === 131);
    assert(count === counter);
  });
  }

  after(async () => {
    await db.close();
  });
});

#pragma once

namespace Utils::Huffman::Tree
{
	struct HuffmanDecompressionNode
	{
		std::uint16_t left{};
		std::uint16_t right{};
	};

	inline constexpr std::array decompressionData
	{
		HuffmanDecompressionNode{ 0, 183 },   // 256
		HuffmanDecompressionNode{ 256, 215 }, // 257
		HuffmanDecompressionNode{ 187, 205 }, // 258
		HuffmanDecompressionNode{ 181, 189 }, // 259
		HuffmanDecompressionNode{ 219, 185 }, // 260
		HuffmanDecompressionNode{ 94, 218 },  // 261
		HuffmanDecompressionNode{ 157, 203 }, // 262
		HuffmanDecompressionNode{ 179, 93 },  // 263
		HuffmanDecompressionNode{ 213, 123 }, // 264
		HuffmanDecompressionNode{ 155, 220 }, // 265
		HuffmanDecompressionNode{ 186, 221 }, // 266
		HuffmanDecompressionNode{ 206, 209 }, // 267
		HuffmanDecompressionNode{ 217, 243 }, // 268
		HuffmanDecompressionNode{ 238, 230 }, // 269
		HuffmanDecompressionNode{ 237, 214 }, // 270
		HuffmanDecompressionNode{ 107, 141 }, // 271
		HuffmanDecompressionNode{ 154, 163 }, // 272
		HuffmanDecompressionNode{ 169, 59 },  // 273
		HuffmanDecompressionNode{ 175, 229 }, // 274
		HuffmanDecompressionNode{ 158, 207 }, // 275
		HuffmanDecompressionNode{ 227, 223 }, // 276
		HuffmanDecompressionNode{ 225, 119 }, // 277
		HuffmanDecompressionNode{ 180, 173 }, // 278
		HuffmanDecompressionNode{ 211, 170 }, // 279
		HuffmanDecompressionNode{ 174, 222 }, // 280
		HuffmanDecompressionNode{ 149, 201 }, // 281
		HuffmanDecompressionNode{ 202, 190 }, // 282
		HuffmanDecompressionNode{ 242, 171 }, // 283
		HuffmanDecompressionNode{ 188, 167 }, // 284
		HuffmanDecompressionNode{ 91, 249 },  // 285
		HuffmanDecompressionNode{ 247, 156 }, // 286
		HuffmanDecompressionNode{ 151, 245 }, // 287
		HuffmanDecompressionNode{ 161, 239 }, // 288
		HuffmanDecompressionNode{ 241, 231 }, // 289
		HuffmanDecompressionNode{ 111, 150 }, // 290
		HuffmanDecompressionNode{ 121, 142 }, // 291
		HuffmanDecompressionNode{ 126, 235 }, // 292
		HuffmanDecompressionNode{ 103, 162 }, // 293
		HuffmanDecompressionNode{ 35, 125 },  // 294
		HuffmanDecompressionNode{ 106, 210 }, // 295
		HuffmanDecompressionNode{ 139, 118 }, // 296
		HuffmanDecompressionNode{ 87, 70 },   // 297
		HuffmanDecompressionNode{ 166, 177 }, // 298
		HuffmanDecompressionNode{ 204, 226 }, // 299
		HuffmanDecompressionNode{ 165, 89 },  // 300
		HuffmanDecompressionNode{ 61, 92 },   // 301
		HuffmanDecompressionNode{ 234, 184 }, // 302
		HuffmanDecompressionNode{ 122, 246 }, // 303
		HuffmanDecompressionNode{ 178, 147 }, // 304
		HuffmanDecompressionNode{ 47, 182 },  // 305
		HuffmanDecompressionNode{ 257, 117 }, // 306
		HuffmanDecompressionNode{ 39, 110 },  // 307
		HuffmanDecompressionNode{ 251, 109 }, // 308
		HuffmanDecompressionNode{ 143, 83 },  // 309
		HuffmanDecompressionNode{ 233, 77 },  // 310
		HuffmanDecompressionNode{ 60, 86 },   // 311
		HuffmanDecompressionNode{ 81, 85 },   // 312
		HuffmanDecompressionNode{ 191, 228 }, // 313
		HuffmanDecompressionNode{ 62, 137 },  // 314
		HuffmanDecompressionNode{ 199, 138 }, // 315
		HuffmanDecompressionNode{ 58, 71 },   // 316
		HuffmanDecompressionNode{ 145, 46 },  // 317
		HuffmanDecompressionNode{ 140, 153 }, // 318
		HuffmanDecompressionNode{ 212, 124 }, // 319
		HuffmanDecompressionNode{ 236, 244 }, // 320
		HuffmanDecompressionNode{ 115, 258 }, // 321
		HuffmanDecompressionNode{ 45, 259 },  // 322
		HuffmanDecompressionNode{ 250, 79 },  // 323
		HuffmanDecompressionNode{ 164, 120 }, // 324
		HuffmanDecompressionNode{ 159, 27 },  // 325
		HuffmanDecompressionNode{ 134, 260 }, // 326
		HuffmanDecompressionNode{ 43, 197 },  // 327
		HuffmanDecompressionNode{ 172, 198 }, // 328
		HuffmanDecompressionNode{ 146, 105 }, // 329
		HuffmanDecompressionNode{ 78, 84 },   // 330
		HuffmanDecompressionNode{ 42, 261 },  // 331
		HuffmanDecompressionNode{ 38, 41 },   // 332
		HuffmanDecompressionNode{ 99, 44 },   // 333
		HuffmanDecompressionNode{ 133, 194 }, // 334
		HuffmanDecompressionNode{ 23, 262 },  // 335
		HuffmanDecompressionNode{ 253, 104 }, // 336
		HuffmanDecompressionNode{ 37, 30 },   // 337
		HuffmanDecompressionNode{ 75, 263 },  // 338
		HuffmanDecompressionNode{ 152, 168 }, // 339
		HuffmanDecompressionNode{ 82, 264 },  // 340
		HuffmanDecompressionNode{ 26, 195 },  // 341
		HuffmanDecompressionNode{ 265, 74 },  // 342
		HuffmanDecompressionNode{ 73, 266 },  // 343
		HuffmanDecompressionNode{ 132, 240 }, // 344
		HuffmanDecompressionNode{ 267, 22 },  // 345
		HuffmanDecompressionNode{ 268, 208 }, // 346
		HuffmanDecompressionNode{ 98, 269 },  // 347
		HuffmanDecompressionNode{ 55, 116 },  // 348
		HuffmanDecompressionNode{ 102, 114 }, // 349
		HuffmanDecompressionNode{ 270, 196 }, // 350
		HuffmanDecompressionNode{ 19, 11 },   // 351
		HuffmanDecompressionNode{ 271, 272 }, // 352
		HuffmanDecompressionNode{ 33, 273 },  // 353
		HuffmanDecompressionNode{ 25, 232 },  // 354
		HuffmanDecompressionNode{ 274, 88 },  // 355
		HuffmanDecompressionNode{ 275, 200 }, // 356
		HuffmanDecompressionNode{ 276, 28 },  // 357
		HuffmanDecompressionNode{ 277, 57 },  // 358
		HuffmanDecompressionNode{ 278, 252 }, // 359
		HuffmanDecompressionNode{ 101, 279 }, // 360
		HuffmanDecompressionNode{ 31, 280 },  // 361
		HuffmanDecompressionNode{ 90, 95 },   // 362
		HuffmanDecompressionNode{ 281, 282 }, // 363
		HuffmanDecompressionNode{ 283, 216 }, // 364
		HuffmanDecompressionNode{ 67, 148 },  // 365
		HuffmanDecompressionNode{ 284, 285 }, // 366
		HuffmanDecompressionNode{ 286, 287 }, // 367
		HuffmanDecompressionNode{ 65, 288 },  // 368
		HuffmanDecompressionNode{ 289, 51 },  // 369
		HuffmanDecompressionNode{ 130, 290 }, // 370
		HuffmanDecompressionNode{ 291, 108 }, // 371
		HuffmanDecompressionNode{ 136, 36 },  // 372
		HuffmanDecompressionNode{ 292, 248 }, // 373
		HuffmanDecompressionNode{ 293, 294 }, // 374
		HuffmanDecompressionNode{ 295, 193 }, // 375
		HuffmanDecompressionNode{ 56, 296 },  // 376
		HuffmanDecompressionNode{ 76, 297 },  // 377
		HuffmanDecompressionNode{ 298, 72 },  // 378
		HuffmanDecompressionNode{ 299, 300 }, // 379
		HuffmanDecompressionNode{ 301, 54 },  // 380
		HuffmanDecompressionNode{ 224, 302 }, // 381
		HuffmanDecompressionNode{ 254, 303 }, // 382
		HuffmanDecompressionNode{ 18, 24 },   // 383
		HuffmanDecompressionNode{ 53, 304 },  // 384
		HuffmanDecompressionNode{ 176, 305 }, // 385
		HuffmanDecompressionNode{ 144, 17 },  // 386
		HuffmanDecompressionNode{ 306, 307 }, // 387
		HuffmanDecompressionNode{ 21, 308 },  // 388
		HuffmanDecompressionNode{ 127, 309 }, // 389
		HuffmanDecompressionNode{ 15, 310 },  // 390
		HuffmanDecompressionNode{ 14, 311 },  // 391
		HuffmanDecompressionNode{ 68, 63 },   // 392
		HuffmanDecompressionNode{ 312, 313 }, // 393
		HuffmanDecompressionNode{ 66, 314 },  // 394
		HuffmanDecompressionNode{ 13, 40 },   // 395
		HuffmanDecompressionNode{ 315, 316 }, // 396
		HuffmanDecompressionNode{ 317, 318 }, // 397
		HuffmanDecompressionNode{ 131, 319 }, // 398
		HuffmanDecompressionNode{ 52, 20 },   // 399
		HuffmanDecompressionNode{ 320, 29 },  // 400
		HuffmanDecompressionNode{ 321, 135 }, // 401
		HuffmanDecompressionNode{ 322, 323 }, // 402
		HuffmanDecompressionNode{ 324, 96 },  // 403
		HuffmanDecompressionNode{ 325, 100 }, // 404
		HuffmanDecompressionNode{ 326, 327 }, // 405
		HuffmanDecompressionNode{ 328, 69 },  // 406
		HuffmanDecompressionNode{ 329, 34 },  // 407
		HuffmanDecompressionNode{ 330, 129 }, // 408
		HuffmanDecompressionNode{ 331, 332 }, // 409
		HuffmanDecompressionNode{ 333, 334 }, // 410
		HuffmanDecompressionNode{ 335, 80 },  // 411
		HuffmanDecompressionNode{ 336, 12 },  // 412
		HuffmanDecompressionNode{ 337, 10 },  // 413
		HuffmanDecompressionNode{ 338, 339 }, // 414
		HuffmanDecompressionNode{ 97, 340 },  // 415
		HuffmanDecompressionNode{ 341, 342 }, // 416
		HuffmanDecompressionNode{ 343, 9 },   // 417
		HuffmanDecompressionNode{ 344, 345 }, // 418
		HuffmanDecompressionNode{ 346, 347 }, // 419
		HuffmanDecompressionNode{ 348, 349 }, // 420
		HuffmanDecompressionNode{ 350, 160 }, // 421
		HuffmanDecompressionNode{ 351, 352 }, // 422
		HuffmanDecompressionNode{ 353, 354 }, // 423
		HuffmanDecompressionNode{ 355, 356 }, // 424
		HuffmanDecompressionNode{ 357, 113 }, // 425
		HuffmanDecompressionNode{ 358, 50 },  // 426
		HuffmanDecompressionNode{ 359, 360 }, // 427
		HuffmanDecompressionNode{ 49, 361 },  // 428
		HuffmanDecompressionNode{ 362, 16 },  // 429
		HuffmanDecompressionNode{ 363, 192 }, // 430
		HuffmanDecompressionNode{ 364, 365 }, // 431
		HuffmanDecompressionNode{ 366, 367 }, // 432
		HuffmanDecompressionNode{ 368, 369 }, // 433
		HuffmanDecompressionNode{ 370, 371 }, // 434
		HuffmanDecompressionNode{ 372, 373 }, // 435
		HuffmanDecompressionNode{ 374, 375 }, // 436
		HuffmanDecompressionNode{ 376, 377 }, // 437
		HuffmanDecompressionNode{ 378, 379 }, // 438
		HuffmanDecompressionNode{ 112, 380 }, // 439
		HuffmanDecompressionNode{ 381, 382 }, // 440
		HuffmanDecompressionNode{ 383, 384 }, // 441
		HuffmanDecompressionNode{ 385, 386 }, // 442
		HuffmanDecompressionNode{ 387, 388 }, // 443
		HuffmanDecompressionNode{ 389, 390 }, // 444
		HuffmanDecompressionNode{ 391, 392 }, // 445
		HuffmanDecompressionNode{ 393, 8 },   // 446
		HuffmanDecompressionNode{ 394, 395 }, // 447
		HuffmanDecompressionNode{ 396, 397 }, // 448
		HuffmanDecompressionNode{ 398, 7 },   // 449
		HuffmanDecompressionNode{ 399, 400 }, // 450
		HuffmanDecompressionNode{ 401, 402 }, // 451
		HuffmanDecompressionNode{ 403, 404 }, // 452
		HuffmanDecompressionNode{ 405, 406 }, // 453
		HuffmanDecompressionNode{ 407, 408 }, // 454
		HuffmanDecompressionNode{ 409, 410 }, // 455
		HuffmanDecompressionNode{ 411, 412 }, // 456
		HuffmanDecompressionNode{ 413, 414 }, // 457
		HuffmanDecompressionNode{ 415, 416 }, // 458
		HuffmanDecompressionNode{ 417, 418 }, // 459
		HuffmanDecompressionNode{ 419, 420 }, // 460
		HuffmanDecompressionNode{ 48, 421 },  // 461
		HuffmanDecompressionNode{ 422, 5 },   // 462
		HuffmanDecompressionNode{ 423, 424 }, // 463
		HuffmanDecompressionNode{ 425, 64 },  // 464
		HuffmanDecompressionNode{ 426, 427 }, // 465
		HuffmanDecompressionNode{ 428, 429 }, // 466
		HuffmanDecompressionNode{ 430, 6 },   // 467
		HuffmanDecompressionNode{ 431, 432 }, // 468
		HuffmanDecompressionNode{ 433, 434 }, // 469
		HuffmanDecompressionNode{ 4, 435 },   // 470
		HuffmanDecompressionNode{ 2, 436 },   // 471
		HuffmanDecompressionNode{ 437, 438 }, // 472
		HuffmanDecompressionNode{ 439, 440 }, // 473
		HuffmanDecompressionNode{ 441, 442 }, // 474
		HuffmanDecompressionNode{ 443, 444 }, // 475
		HuffmanDecompressionNode{ 445, 446 }, // 476
		HuffmanDecompressionNode{ 447, 128 }, // 477
		HuffmanDecompressionNode{ 3, 448 },   // 478
		HuffmanDecompressionNode{ 449, 450 }, // 479
		HuffmanDecompressionNode{ 32, 451 },  // 480
		HuffmanDecompressionNode{ 452, 453 }, // 481
		HuffmanDecompressionNode{ 454, 455 }, // 482
		HuffmanDecompressionNode{ 456, 457 }, // 483
		HuffmanDecompressionNode{ 255, 458 }, // 484
		HuffmanDecompressionNode{ 459, 1 },   // 485
		HuffmanDecompressionNode{ 460, 461 }, // 486
		HuffmanDecompressionNode{ 462, 463 }, // 487
		HuffmanDecompressionNode{ 464, 465 }, // 488
		HuffmanDecompressionNode{ 466, 467 }, // 489
		HuffmanDecompressionNode{ 468, 469 }, // 490
		HuffmanDecompressionNode{ 470, 471 }, // 491
		HuffmanDecompressionNode{ 472, 473 }, // 492
		HuffmanDecompressionNode{ 474, 475 }, // 493
		HuffmanDecompressionNode{ 476, 477 }, // 494
		HuffmanDecompressionNode{ 478, 479 }, // 495
		HuffmanDecompressionNode{ 480, 481 }, // 496
		HuffmanDecompressionNode{ 482, 483 }, // 497
		HuffmanDecompressionNode{ 484, 485 }, // 498
		HuffmanDecompressionNode{ 486, 487 }, // 499
		HuffmanDecompressionNode{ 488, 489 }, // 500
		HuffmanDecompressionNode{ 490, 491 }, // 501
		HuffmanDecompressionNode{ 492, 493 }, // 502
		HuffmanDecompressionNode{ 494, 495 }, // 503
		HuffmanDecompressionNode{ 496, 497 }, // 504
		HuffmanDecompressionNode{ 498, 499 }, // 505
		HuffmanDecompressionNode{ 500, 501 }, // 506
		HuffmanDecompressionNode{ 502, 503 }, // 507
		HuffmanDecompressionNode{ 504, 0 },   // 508
		HuffmanDecompressionNode{ 505, 506 }, // 509
		HuffmanDecompressionNode{ 507, 508 }, // 510
		HuffmanDecompressionNode{ 509, 510 }  // 511
	};

	struct HuffmanCompressionNode
	{
		std::array<std::uint8_t, 12> nodeData{};
	};

	inline constexpr std::array compressionData
	{
		HuffmanCompressionNode{ 3, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },  // 0
		HuffmanCompressionNode{ 5, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0 },  // 1
		HuffmanCompressionNode{ 6, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },  // 2
		HuffmanCompressionNode{ 6, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },  // 3
		HuffmanCompressionNode{ 6, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },  // 4
		HuffmanCompressionNode{ 6, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0 },  // 5
		HuffmanCompressionNode{ 6, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0 },  // 6
		HuffmanCompressionNode{ 7, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0 },  // 7
		HuffmanCompressionNode{ 7, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0 },  // 8
		HuffmanCompressionNode{ 7, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0 },  // 9
		HuffmanCompressionNode{ 8, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0 },  // 10
		HuffmanCompressionNode{ 8, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0 },  // 11
		HuffmanCompressionNode{ 8, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0 },  // 12
		HuffmanCompressionNode{ 8, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0 },  // 13
		HuffmanCompressionNode{ 8, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },  // 14
		HuffmanCompressionNode{ 8, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 },  // 15
		HuffmanCompressionNode{ 7, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0 },  // 16
		HuffmanCompressionNode{ 8, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0 },  // 17
		HuffmanCompressionNode{ 8, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },  // 18
		HuffmanCompressionNode{ 8, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },  // 19
		HuffmanCompressionNode{ 8, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0 },  // 20
		HuffmanCompressionNode{ 8, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0 },  // 21
		HuffmanCompressionNode{ 8, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0 },  // 22
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0 },  // 23
		HuffmanCompressionNode{ 8, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },  // 24
		HuffmanCompressionNode{ 8, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0 },  // 25
		HuffmanCompressionNode{ 8, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 },  // 26
		HuffmanCompressionNode{ 9, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0 },  // 27
		HuffmanCompressionNode{ 8, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0 },  // 28
		HuffmanCompressionNode{ 8, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0 },  // 29
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0 },  // 30
		HuffmanCompressionNode{ 8, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0 },  // 31
		HuffmanCompressionNode{ 6, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  // 32
		HuffmanCompressionNode{ 8, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0 },  // 33
		HuffmanCompressionNode{ 8, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0 },  // 34
		HuffmanCompressionNode{ 9, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0 },  // 35
		HuffmanCompressionNode{ 8, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0 },  // 36
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0 },  // 37
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0 },  // 38
		HuffmanCompressionNode{ 9, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0 },  // 39
		HuffmanCompressionNode{ 8, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0 },  // 40
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0 },  // 41
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 },  // 42
		HuffmanCompressionNode{ 9, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0 },  // 43
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0 },  // 44
		HuffmanCompressionNode{ 9, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0 },  // 45
		HuffmanCompressionNode{ 9, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0 },  // 46
		HuffmanCompressionNode{ 9, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0 },  // 47
		HuffmanCompressionNode{ 6, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0 },  // 48
		HuffmanCompressionNode{ 7, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },  // 49
		HuffmanCompressionNode{ 7, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0 },  // 50
		HuffmanCompressionNode{ 8, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0 },  // 51
		HuffmanCompressionNode{ 8, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0 },  // 52
		HuffmanCompressionNode{ 8, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0 },  // 53
		HuffmanCompressionNode{ 8, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0 },  // 54
		HuffmanCompressionNode{ 8, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0 },  // 55
		HuffmanCompressionNode{ 8, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  // 56
		HuffmanCompressionNode{ 8, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0 },  // 57
		HuffmanCompressionNode{ 9, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0 },  // 58
		HuffmanCompressionNode{ 9, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0 },  // 59
		HuffmanCompressionNode{ 9, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 },  // 60
		HuffmanCompressionNode{ 9, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 },  // 61
		HuffmanCompressionNode{ 9, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0 },  // 62
		HuffmanCompressionNode{ 8, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0 },  // 63
		HuffmanCompressionNode{ 6, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0 },  // 64
		HuffmanCompressionNode{ 8, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0 },  // 65
		HuffmanCompressionNode{ 8, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0 },  // 66
		HuffmanCompressionNode{ 8, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 },  // 67
		HuffmanCompressionNode{ 8, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 },  // 68
		HuffmanCompressionNode{ 8, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0 },  // 69
		HuffmanCompressionNode{ 9, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0 },  // 70
		HuffmanCompressionNode{ 9, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0 },  // 71
		HuffmanCompressionNode{ 8, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0 },  // 72
		HuffmanCompressionNode{ 8, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },  // 73
		HuffmanCompressionNode{ 8, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0 },  // 74
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0 },  // 75
		HuffmanCompressionNode{ 8, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },  // 76
		HuffmanCompressionNode{ 9, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0 },  // 77
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0 },  // 78
		HuffmanCompressionNode{ 9, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0 },  // 79
		HuffmanCompressionNode{ 8, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0 },  // 80
		HuffmanCompressionNode{ 9, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0 },  // 81
		HuffmanCompressionNode{ 8, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 },  // 82
		HuffmanCompressionNode{ 9, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0 },  // 83
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0 },  // 84
		HuffmanCompressionNode{ 9, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0 },  // 85
		HuffmanCompressionNode{ 9, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0 },  // 86
		HuffmanCompressionNode{ 9, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0 },  // 87
		HuffmanCompressionNode{ 8, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0 },  // 88
		HuffmanCompressionNode{ 9, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0 },  // 89
		HuffmanCompressionNode{ 8, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 },  // 90
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0 },  // 91
		HuffmanCompressionNode{ 9, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0 },  // 92
		HuffmanCompressionNode{ 10, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0 }, // 93
		HuffmanCompressionNode{ 10, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0 }, // 94
		HuffmanCompressionNode{ 8, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0 },  // 95
		HuffmanCompressionNode{ 8, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0 },  // 96
		HuffmanCompressionNode{ 7, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },  // 97
		HuffmanCompressionNode{ 8, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 },  // 98
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0 },  // 99
		HuffmanCompressionNode{ 8, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0 },  // 100
		HuffmanCompressionNode{ 8, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0 },  // 101
		HuffmanCompressionNode{ 8, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0 },  // 102
		HuffmanCompressionNode{ 9, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },  // 103
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0 },  // 104
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0 },  // 105
		HuffmanCompressionNode{ 9, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },  // 106
		HuffmanCompressionNode{ 9, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0 },  // 107
		HuffmanCompressionNode{ 8, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0 },  // 108
		HuffmanCompressionNode{ 9, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0 },  // 109
		HuffmanCompressionNode{ 9, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0 },  // 110
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0 },  // 111
		HuffmanCompressionNode{ 7, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },  // 112
		HuffmanCompressionNode{ 7, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 },  // 113
		HuffmanCompressionNode{ 8, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0 },  // 114
		HuffmanCompressionNode{ 9, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0 },  // 115
		HuffmanCompressionNode{ 8, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0 },  // 116
		HuffmanCompressionNode{ 9, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0 },  // 117
		HuffmanCompressionNode{ 9, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0 },  // 118
		HuffmanCompressionNode{ 9, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0 },  // 119
		HuffmanCompressionNode{ 9, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0 },  // 120
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0 },  // 121
		HuffmanCompressionNode{ 9, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0 },  // 122
		HuffmanCompressionNode{ 9, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0 },  // 123
		HuffmanCompressionNode{ 9, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0 },  // 124
		HuffmanCompressionNode{ 9, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0 },  // 125
		HuffmanCompressionNode{ 9, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0 },  // 126
		HuffmanCompressionNode{ 8, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0 },  // 127
		HuffmanCompressionNode{ 6, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0 },  // 128
		HuffmanCompressionNode{ 8, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0 },  // 129
		HuffmanCompressionNode{ 8, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0 },  // 130
		HuffmanCompressionNode{ 8, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0 },  // 131
		HuffmanCompressionNode{ 8, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0 },  // 132
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0 },  // 133
		HuffmanCompressionNode{ 9, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0 },  // 134
		HuffmanCompressionNode{ 8, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0 },  // 135
		HuffmanCompressionNode{ 8, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0 },  // 136
		HuffmanCompressionNode{ 9, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0 },  // 137
		HuffmanCompressionNode{ 9, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0 },  // 138
		HuffmanCompressionNode{ 9, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },  // 139
		HuffmanCompressionNode{ 9, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0 },  // 140
		HuffmanCompressionNode{ 9, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0 },  // 141
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0 },  // 142
		HuffmanCompressionNode{ 9, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0 },  // 143
		HuffmanCompressionNode{ 8, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0 },  // 144
		HuffmanCompressionNode{ 9, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0 },  // 145
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },  // 146
		HuffmanCompressionNode{ 9, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0 },  // 147
		HuffmanCompressionNode{ 8, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0 },  // 148
		HuffmanCompressionNode{ 9, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0 },  // 149
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0 },  // 150
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0 },  // 151
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0 },  // 152
		HuffmanCompressionNode{ 9, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0 },  // 153
		HuffmanCompressionNode{ 9, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0 },  // 154
		HuffmanCompressionNode{ 9, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0 },  // 155
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0 },  // 156
		HuffmanCompressionNode{ 10, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0 }, // 157
		HuffmanCompressionNode{ 9, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },  // 158
		HuffmanCompressionNode{ 9, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0 },  // 159
		HuffmanCompressionNode{ 7, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0 },  // 160
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0 },  // 161
		HuffmanCompressionNode{ 9, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0 },  // 162
		HuffmanCompressionNode{ 9, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0 },  // 163
		HuffmanCompressionNode{ 9, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0 },  // 164
		HuffmanCompressionNode{ 9, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0 },  // 165
		HuffmanCompressionNode{ 9, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 },  // 166
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0 },  // 167
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0 },  // 168
		HuffmanCompressionNode{ 9, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0 },  // 169
		HuffmanCompressionNode{ 9, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0 },  // 170
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0 },  // 171
		HuffmanCompressionNode{ 9, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0 },  // 172
		HuffmanCompressionNode{ 9, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0 },  // 173
		HuffmanCompressionNode{ 9, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0 },  // 174
		HuffmanCompressionNode{ 9, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0 },  // 175
		HuffmanCompressionNode{ 8, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0 },  // 176
		HuffmanCompressionNode{ 9, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0 },  // 177
		HuffmanCompressionNode{ 9, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0 },  // 178
		HuffmanCompressionNode{ 10, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0 }, // 179
		HuffmanCompressionNode{ 9, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0 },  // 180
		HuffmanCompressionNode{ 10, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0 }, // 181
		HuffmanCompressionNode{ 9, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0 },  // 182
		HuffmanCompressionNode{ 11, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1 }, // 183
		HuffmanCompressionNode{ 9, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0 },  // 184
		HuffmanCompressionNode{ 10, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0 }, // 185
		HuffmanCompressionNode{ 9, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },  // 186
		HuffmanCompressionNode{ 10, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0 }, // 187
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0 },  // 188
		HuffmanCompressionNode{ 10, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0 }, // 189
		HuffmanCompressionNode{ 9, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0 },  // 190
		HuffmanCompressionNode{ 9, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0 },  // 191
		HuffmanCompressionNode{ 7, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0 },  // 192
		HuffmanCompressionNode{ 8, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0 },  // 193
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0 },  // 194
		HuffmanCompressionNode{ 8, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0 },  // 195
		HuffmanCompressionNode{ 8, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0 },  // 196
		HuffmanCompressionNode{ 9, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0 },  // 197
		HuffmanCompressionNode{ 9, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0 },  // 198
		HuffmanCompressionNode{ 9, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0 },  // 199
		HuffmanCompressionNode{ 8, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0 },  // 200
		HuffmanCompressionNode{ 9, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0 },  // 201
		HuffmanCompressionNode{ 9, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0 },  // 202
		HuffmanCompressionNode{ 10, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0 }, // 203
		HuffmanCompressionNode{ 9, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 },  // 204
		HuffmanCompressionNode{ 10, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0 }, // 205
		HuffmanCompressionNode{ 9, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0 },  // 206
		HuffmanCompressionNode{ 9, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0 },  // 207
		HuffmanCompressionNode{ 8, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 },  // 208
		HuffmanCompressionNode{ 9, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0 },  // 209
		HuffmanCompressionNode{ 9, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0 },  // 210
		HuffmanCompressionNode{ 9, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0 },  // 211
		HuffmanCompressionNode{ 9, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0 },  // 212
		HuffmanCompressionNode{ 9, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0 },  // 213
		HuffmanCompressionNode{ 9, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0 },  // 214
		HuffmanCompressionNode{ 10, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0 }, // 215
		HuffmanCompressionNode{ 8, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0 },  // 216
		HuffmanCompressionNode{ 9, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },  // 217
		HuffmanCompressionNode{ 10, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0 }, // 218
		HuffmanCompressionNode{ 10, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0 }, // 219
		HuffmanCompressionNode{ 9, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0 },  // 220
		HuffmanCompressionNode{ 9, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0 },  // 221
		HuffmanCompressionNode{ 9, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0 },  // 222
		HuffmanCompressionNode{ 9, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0 },  // 223
		HuffmanCompressionNode{ 8, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 },  // 224
		HuffmanCompressionNode{ 9, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0 },  // 225
		HuffmanCompressionNode{ 9, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0 },  // 226
		HuffmanCompressionNode{ 9, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  // 227
		HuffmanCompressionNode{ 9, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0 },  // 228
		HuffmanCompressionNode{ 9, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0 },  // 229
		HuffmanCompressionNode{ 9, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0 },  // 230
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0 },  // 231
		HuffmanCompressionNode{ 8, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0 },  // 232
		HuffmanCompressionNode{ 9, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0 },  // 233
		HuffmanCompressionNode{ 9, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0 },  // 234
		HuffmanCompressionNode{ 9, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0 },  // 235
		HuffmanCompressionNode{ 9, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },  // 236
		HuffmanCompressionNode{ 9, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0 },  // 237
		HuffmanCompressionNode{ 9, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0 },  // 238
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0 },  // 239
		HuffmanCompressionNode{ 8, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0 },  // 240
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0 },  // 241
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },  // 242
		HuffmanCompressionNode{ 9, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0 },  // 243
		HuffmanCompressionNode{ 9, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0 },  // 244
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0 },  // 245
		HuffmanCompressionNode{ 9, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0 },  // 246
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0 },  // 247
		HuffmanCompressionNode{ 8, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0 },  // 248
		HuffmanCompressionNode{ 9, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0 },  // 249
		HuffmanCompressionNode{ 9, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0 },  // 250
		HuffmanCompressionNode{ 9, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0 },  // 251
		HuffmanCompressionNode{ 8, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0 },  // 252
		HuffmanCompressionNode{ 9, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0 },  // 253
		HuffmanCompressionNode{ 8, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0 },  // 254
		HuffmanCompressionNode{ 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }   // 255
	};

	static_assert(decompressionData.size() == 256 && compressionData.size() == 256);
}

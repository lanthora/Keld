// SPDX-License-Identifier: GPL-2.0-only
package render

import (
	"net/http"

	"github.com/gin-gonic/gin"
)

const (
	StatusSuccess = iota + 0
	StatusUnknownError
	StatusInvalidArgument
)

const (
	StatusUserNotLoggedIn = iota + 100
	StatusUserPermissionDenied
	StatusUserLoginFaild
	StatusUserCreateUserFailed
	StatusUserQueryUserFailed
	StatusUserUpdateUserFailed
	StatusUserDeleteUserFailed
)

const (
	StatusProcessEnableFailed = iota + 200
	StatusProcessDisableFailed
	StatusProcessUpdateJudgeFailed
	StatusProcessUpdatePolicyFailed
	StatusProcessQueryEventFailed
	StatusProcessTrustUpdateFailed
	StatusProcessGetTrustStatusFailed
)

const (
	StatusFileEnableFailed = iota + 300
	StatusFileDisableFailed
	StatusFileAddPolicyConflict
	StatusFileAddPolicyFileNotExist
	StatusFileAddPolicyFailed
	StatusFileDeletePolicyFailed
	StatusFileQueryPolicyListFailed
	StatusFileQueryPolicyByIdFailed
	StatusFileQueryEventListFailed
	StatusFileDeleteEventFailed
	StatusFileUpdatePolicyConflict
	StatusFileUpdatePolicyFileNotExist
	StatusFileUpdatePolicyFailed
	StatusFileUpdateEventStatusFailed
	StatusFileQueryEventFailed
	StatusFileBrowseFailed
)

const (
	StatusNetEnableFailed = iota + 400
	StatusNetDisableFailed
	StatusNetAddPolicyFailed
	StatusNetAddPolicyDatabaseFailed
	StatusNetDeletePolicyFailed
	StatusNetDeletePolicyDatabaseFailed
	StatusNetQueryPolicyListFailed
	StatusNetPolicyNotExist
	StatusNetQueryEventFailed
	StatusNetDeleteEventFailed
	StatusNetUpdateEventStatusFailed
)

var messages = map[int]string{
	StatusFileBrowseFailed:              "Cannot read this directory. Check that it exists and that the service can access it.",
	StatusSuccess:                       "Success",
	StatusUnknownError:                  "Unknown error",
	StatusInvalidArgument:               "Invalid parameters",
	StatusUserNotLoggedIn:               "Not signed in",
	StatusUserPermissionDenied:          "Permission denied",
	StatusUserLoginFaild:                "Sign-in failed",
	StatusUserCreateUserFailed:          "Could not create user",
	StatusUserQueryUserFailed:           "Could not query users",
	StatusUserUpdateUserFailed:          "Could not update user",
	StatusUserDeleteUserFailed:          "Could not delete user",
	StatusProcessEnableFailed:           "Could not enable process protection",
	StatusProcessDisableFailed:          "Could not disable process protection",
	StatusProcessUpdateJudgeFailed:      "Could not update the process protection mode",
	StatusProcessUpdatePolicyFailed:     "Could not update the process policy",
	StatusProcessQueryEventFailed:       "Could not query process events",
	StatusProcessTrustUpdateFailed:      "Could not update the default command trust status",
	StatusProcessGetTrustStatusFailed:   "Could not get the default command trust status",
	StatusFileEnableFailed:              "Could not enable file protection",
	StatusFileDisableFailed:             "Could not disable file protection",
	StatusFileAddPolicyConflict:         "The file policy conflicts with an existing policy",
	StatusFileAddPolicyFileNotExist:     "The file for this policy does not exist",
	StatusFileAddPolicyFailed:           "Could not add the file policy",
	StatusFileDeletePolicyFailed:        "Could not delete the file policy",
	StatusFileQueryPolicyListFailed:     "Could not query file policies",
	StatusFileQueryPolicyByIdFailed:     "Could not query the file policy",
	StatusFileQueryEventListFailed:      "Could not query file events",
	StatusFileDeleteEventFailed:         "Could not delete the file event",
	StatusFileUpdatePolicyConflict:      "The updated file policy conflicts with an existing policy",
	StatusFileUpdatePolicyFileNotExist:  "The file for the updated policy does not exist",
	StatusFileUpdatePolicyFailed:        "Could not update the file policy",
	StatusFileUpdateEventStatusFailed:   "Could not update the file event status",
	StatusFileQueryEventFailed:          "Could not query file events",
	StatusNetEnableFailed:               "Could not enable network protection",
	StatusNetDisableFailed:              "Could not disable network protection",
	StatusNetAddPolicyFailed:            "Could not add the network policy",
	StatusNetAddPolicyDatabaseFailed:    "Could not save the network policy",
	StatusNetDeletePolicyFailed:         "Could not delete the network policy",
	StatusNetDeletePolicyDatabaseFailed: "Could not delete the saved network policy",
	StatusNetQueryPolicyListFailed:      "Could not query network policies",
	StatusNetPolicyNotExist:             "Network policy not found",
	StatusNetQueryEventFailed:           "Could not query network events",
	StatusNetDeleteEventFailed:          "Could not delete the network event",
	StatusNetUpdateEventStatusFailed:    "Could not update the network event status",
}

func Success(context *gin.Context, data interface{}) {
	response := struct {
		Status  int         `json:"status"`
		Message string      `json:"message"`
		Data    interface{} `json:"data"`
	}{
		Status:  StatusSuccess,
		Message: messages[StatusSuccess],
		Data:    data,
	}
	context.JSON(http.StatusOK, response)
}

func Status(context *gin.Context, status int) {
	response := struct {
		Status  int    `json:"status"`
		Message string `json:"message"`
	}{
		Status:  status,
		Message: messages[status],
	}
	context.JSON(http.StatusOK, response)
}
